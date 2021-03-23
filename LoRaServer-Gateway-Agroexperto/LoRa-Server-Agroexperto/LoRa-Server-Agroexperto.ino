//SERVER (Gateway remoto)
//placa Heltec LoRa v2

#include <HTTPClient.h>
#include <DHT.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
#define RELAY 13
#define DHTTYPE DHT22  //Define o tipo de sensor DHT
#define DHTPIN 12   //12 Pino de ligacao do DHT22
#define PIN_VL 36  // 36  Leitura do  Pluviômetro
#define PL 37     //37  RST Pluviometro
//observar que os pinos 2, 4, 5, 14, 15, 16, 19, 18, 21, 26, 27 não podem ser usados

//SSID e senha do endpoint
 const char* ssid = "teste1";
  const char* password = "12345678";
  IPAddress staticIP(192,168,5, 199); //IP do REMOTE
  IPAddress gateway ( 192, 168, 1, 1);
  IPAddress subnet ( 255, 255, 255, 0 );
  IPAddress server(192, 168, 4, 151);
  const char* serverNameTemp = "";
  

//define e inicializa as variáveis
int minima = 99;
int maxima = 0;
int t;
int h;
int h_ant = 0;
int t_ant = 0;
int counter = 0;
bool vl;
const long intervalo = 60000; //Intervalo de tempo entre leituras
unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura
unsigned long lastDebounceTime = 0;
const long debounceDelay = 100;
bool lastState = 0;
int reedCounter = 0;
float precipitacao = 0.0;

//Instancia o sensor
DHT dht(DHTPIN, DHTTYPE);

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

// Porta do server que vc vai utilizar para conectar no endpoint
const int port = 80; 

//Instancia cliente wifi
WiFiClient client;

//Tarefas para verificar novos clientes e mensagens enviadas por estes
//Scheduler scheduler;
//void handleWeather();
//void taskHandleClient();
////Tarefa para verificar se uma nova conexão feita por aplicativo está sendo feita
//Task t1(100, TASK_FOREVER, &handleWeather, &scheduler, true);
////Tarefa para verificar se há novas mensagens vindas de aplicativo
//Task t2(100, TASK_FOREVER, &taskHandleClient, &scheduler, true);

//Id e estados deste esp (altere para cada esp)
String ID = "REMOTE1"; // ID desta unidade
String ID_ON = ID + " ON";
String ID_OFF = ID + " OFF";
//identificação da estação (código AgroexPerto)
String stationCode = "SP200"; //id da estação - alterar e conferir depois de depurar o programa


//Variável para guardar o valor do estado atual do relê 
String currentState = ID_OFF;

void setup() {
        //Coloca tudo em maiúsculo
        ID_ON.toUpperCase();
        ID_OFF.toUpperCase();
      
        //Coloca o pino onde o relê está como saída
        pinMode(RELAY, OUTPUT);
      
        Heltec.begin(true /*Ativa o display*/, true /*Ativa lora*/, true /*Ativa informações pela serial*/, true /*Ativa PABOOST*/, BAND /*frequência*/);
       
        //Inicializa o display
        setupDisplay();
      
        //Ativa o recebimento e envio de pacotes lora
        LoRa.receive();
      
        //Se conecta à rede WiFi
        setupWiFi();  
      
        //Modifica o estado do relê para desligado
        verifyDestiny(ID_OFF);
      
        //Inicializa o agendador de tarefas
       // scheduler.startNow();
      }

//Inicializa o display
void setupDisplay() {
        Heltec.display->init();
        //Limpa o display
        Heltec.display->clear();
        //Modifica direcionamento do texto
        Heltec.display->flipScreenVertically();  
        //Alinha o texto à esquerda
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        //Altera a fonte
        Heltec.display->setFont(ArialMT_Plain_16);
        //Exibe no display
        Heltec.display->drawString(0, 0, "Display OK");
        Heltec.display->display();
        }

void setupWiFi() {
        Serial.print("Conectando");    
        //Faz o ESP se conectar à rede WiFi
        WiFi.config (staticIP, gateway, subnet);
        WiFi.begin(ssid, password);    
        //Enquanto o ESP não se conectar à rede
        byte count = 0;
          while (WiFi.status() != WL_CONNECTED && count < 50) {
              count ++; //fazemos "count" tentativas
              //Esperamos 100 milisegundos
              delay(100);
              Serial.print(".");
              }
        //Se chegou aqui é porque conectou à rede, então mostramos no monitor serial para termos um feedback
        Serial.println("");
        Serial.println("Conectou");    
        //Objeto que vamos utilizar para guardar o ip recebido
        myIP = WiFi.localIP();
        //Mostra o ip no monitor serial
        Serial.println(myIP);    
        //Atualiza o display para exibir o ip
        refreshDisplay();
      }

void refreshDisplay() {
        //Limpa o display
        Heltec.display->clear();
        //Exibe o estado atual do relê
        Heltec.display->drawString(0, 0, currentState);
        //Exibe o ip deste esp para ser utilizado no aplicativo
        Heltec.display->drawString(0, 15, myIP.toString());
        Heltec.display->display();
      }

void weatherDisplay(int temperatura, int max_s, int min_s){
      //Limpa o display
        Heltec.display->clear();
        //Atualiza informacoes da temperatura
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 2, "AgroexPerto " + stationCode);
        Heltec.display->setFont(ArialMT_Plain_24);
        Heltec.display->drawString(32, 26, String(temperatura));
        Heltec.display->drawCircle(52, 32, 2);      
        //Atualiza maxima 
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(73, 24, "Max");
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(101, 19, String(max_s));
        //Atualiza minima      
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(73, 48, "Min");
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(101, 43, String(min_s));
        Heltec.display->display();
}

void loop() {
    readPluv();  
     unsigned long currentMillis = millis();       
        if (currentMillis - previousMillis >= intervalo){  //Verifica se o intervalo já foi atingido
           previousMillis = currentMillis; //Armazena o valor da ultima leitura
          handleWeather();
        }        
     //Faz a leitura do pacote
      String packet = readLoRaPacket();
      Serial.println("1 " + packet);
      //Se uma mensagem chegou
      if(!packet.equals("")) {
      handleCommand(packet);
      }
      //Executa as tarefas que foram adicionadas ao scheduler
      //scheduler.execute();

      
  }

//Faz a leitura de um pacote (se chegou algum)
String readLoRaPacket() {
      String packet = "";
      //Verifica o tamanho do pacote
      int packetSize = LoRa.parsePacket();
      //Lê cada caractere e concatena na string 
      for (int i = 0; i < packetSize; i++) { 
        packet += (char) LoRa.read(); 
      }
      return packet;
    }

// Função que verifica o comando vindo do gateway
void handleCommand(String cmd){
      // Se a String estiver vazia não precisamos fazer nada
      if (cmd.equals(""))
        return;         
      // Exibimos o comando recebido no monitor serial
      delay(5000);
      Serial.println("2 Received from Gateway: " + cmd);    
      //Verifica se a mensagem é para este esp
      bool forMe = verifyDestiny(cmd);    
      //Se a mensagem é para este esp
      if(forMe) {
        if (cmd.indexOf("rootRelay") != -1) {             
            digitalWrite(RELAY, (cmd.indexOf("Off")) ? LOW : HIGH); //muda o estado do relay
              //Atualizamos o display com o estado atualizado
              //Envia mensagem de confirmaçao de volta para o gateway
              String confirmationMessage = ID + " OK";
              sendLoRaPacket(confirmationMessage);
              Serial.println("4 Changed Relay status: " + confirmationMessage);
            }
        //Se  é para o endpoint
        else {
          //Envia o comando para o endpoint
          digitalWrite(25, HIGH);
          Serial.println("4 comando deve ser enviado para o endpoint");
          delay(5000);
         sendToEnd(cmd);
        }
      }
    }

  //Verifica se estado é valido para este esp e modifica o estado do relê de acordo
//Retorna true se a mensagem for para este esp e false caso contrário
bool verifyDestiny(String state) {
      //Se a mudança de estado pertence ao id vinculado a este esp
      if (state.indexOf("REMOTE1") != -1) {          
              refreshDisplay();
              delay(5000);
              Serial.println("3 o comando é para este esp"); 
              return true;                       
               }
       return false;
       Serial.println("3 o comando não é para este esp");             
       }

//Função que envia mensagem para o endpoint - module relay
void sendToEnd(String msg) {
  const char* host = "http://192.168.4.151/";
  if ((WiFi.status() == WL_CONNECTED)){
    
    Serial.println("5 sendToEnd inciado");   
    String endPointcmd = host + msg;
    delay(5000);
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    // This will send the request to the server
    client.print(String("GET ") + msg + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
     while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    
//    HTTPClient http;
//    http.begin(endPointcmd);
//    Serial.println("6 Tentando enviar ao endpoint");
//    delay(5000);    
//        int httpCode = http.GET();
//        if(httpCode>0){
//            Serial.println("7 comando enviado ao endpoint");
//            Serial.println(httpCode);
//            delay(5000);
//            return;       
//            }
//         else {
//          Serial.println("7 sending to endpoint failed");
//         }
    }
}
  


//Envia um pacote LoRa
void sendLoRaPacket(String str) {
        //Inicializa o pacote
        LoRa.beginPacket();
        //Coloca a string no pacote
        LoRa.print(str);
        //Finaliza e envia o pacote
        LoRa.endPacket();
        delay(5000);
        Serial.println("7  " + str);
      }

void handleWeather(){
  //executa a rotina de coleta do DHT                                
              counter = counter + 1; //acrescenta no contador de leituras
              Serial.println(counter); //mostra no monitor
              //Le a temperatura e a umidade do ar
              t = dht.readTemperature();
              h = dht.readHumidity();
              
                if (t > 100) {
                  t = t_ant;}    //para evitar picos do sensor
                if (h > 100) {
                  h = h_ant;}   //para evitar picos do sensor
          
              //Mostra a temperatura e umidade no Serial Monitor
              Serial.print(F("Temperatura: "));
              Serial.print(t);
              Serial.println(F(" °C "));
              Serial.print(F("Umidade Relativa: "));
              Serial.print(h);
              Serial.println(" %");
              
                //Atualiza as variaveis maxima e minima, se necessario
                
                if (t >= maxima){
                  maxima = t;}
                if (t <= minima){
                  minima = t;}  
              
              //Envia as informacoes para o display
              weatherDisplay(t, maxima, minima);  
          //===============================================================
          
                    if(counter >= 60){
                      counter = 0;            
                      
                      //Cria a string com os dados
                      String body = "leit1=" + stationCode + "&leit2=" + t + "&leit3=" + h + "&leit4=" + precipitacao;
                    
                            sendLoRaPacket(body);                
                          }            
                 t_ant = t;
                 h_ant = h;
                 precipitacao=0;
                }

void readPluv(){
      // Contagem pulsos pluviometro
        vl = digitalRead(PIN_VL);  // Inicia a leitura do pluviom            
        if ( vl == 1 && lastState == 0 ) {
          Serial.println(" bascula acionada ");
          lastDebounceTime = millis();
          lastState = 1;
          }    
        if ((millis() - lastDebounceTime) > debounceDelay) {
          if (lastState == 1 && vl == 0){
             Serial.println("nova leitura");
            reedCounter +=1;
            lastState = 0;
            lastDebounceTime = 0;
            precipitacao = float(reedCounter)*0.27;        
            Serial.print("Precipitação (mm)= ");
            Serial.print(precipitacao);
            Serial.println(" mm");
          }
        }  
  }
      
