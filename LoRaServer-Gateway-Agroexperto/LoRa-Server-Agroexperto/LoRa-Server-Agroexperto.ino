
//SERVER (Gateway remoto)

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
#define DHTPIN 5   //P5 D1 Pino de ligacao do DHT22
#define PIN_VL 4  // P4  D2  Leitura do  Pluviômetro
#define PL 12     //P12  D6  RST Pluviometro
//observar que o LCD está nos pinos P2 e P14

//SSID e senha do endpoint
 const char* ssid = "xpertinho";
  const char* password = "xpert-secret";
  IPAddress staticIP(192,168,1, 200); //IP do REMOTE
  IPAddress gateway ( 192, 168, 1, 1);
  IPAddress subnet ( 255, 255, 255, 0 );
  IPAddress server(192, 168, 4, 15);

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
String ID = "REMOTE1";
String ID_ON = ID + " ON";
String ID_OFF = ID + " OFF";
//identificação da estação (código AgroexPerto)
String stationCode = "SP200";


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
        verifyAndSetRelayState(ID_OFF);
      
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
          while (WiFi.status() != WL_CONNECTED) {
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
     unsigned long currentMillis = millis();       
        if (currentMillis - previousMillis >= intervalo){  //Verifica se o intervalo já foi atingido
           previousMillis = currentMillis; //Armazena o valor da ultima leitura
          handleWeather();
        }
        
  //Faz a leitura do pacote
      String packet = readLoRaPacket();
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
void handleCommand(String cmd)
    {
      // Se a String estiver vazia não precisamos fazer nada
      if (cmd.equals(""))
        return;
      //Coloca todos os caracteres em maiúsculo
      cmd.toUpperCase();    
      // Exibimos o comando recebido no monitor serial
      Serial.println("Received from app: " + cmd);    
      //Verifica se a mensagem é para este esp e modifica o estado do relê de acordo com o que foi enviado
      bool forMe = verifyAndSetRelayState(cmd);    
      //Se a mensagem é para este esp
      if(forMe) {
        //Envia mensagem de confirmaçao de volta para o gateway
        String confirmationMessage = currentState + " OK";
        sendLoRaPacket(confirmationMessage);
        Serial.println("Changed Relay status: " + confirmationMessage);
      }
      //Se  é para o endpoint
      else {
        //Envia o comando para o endpoint
       sendToClient(cmd);
      }
  }

  //Verifica se estado é valido para este esp e modifica o estado do relê de acordo
//Retorna true se a mensagem for para este esp e false caso contrário
bool verifyAndSetRelayState(String state) {
      //Se a mudança de estado pertence ao id vinculado a este esp
      if (state == ID_ON || state == ID_OFF) {
            //Guarda o estado atual
            currentState = state;
        
            //Modificamos o estado do relê de acordo com o estado enviado
            digitalWrite(RELAY, currentState == ID_ON ? LOW : HIGH);
            
            //Atualizamos o display com o estado atualizado
            refreshDisplay();
            return true;
        }
      return false;
  }

//Função que envia mensagem para o endpoint
void sendToClient(String msg) {
   client.print(msg);
  }


//Envia um pacote LoRa
void sendLoRaPacket(String str) {
  //Inicializa o pacote
  LoRa.beginPacket();
  //Coloca a string no pacote
  LoRa.print(str);
  //Finaliza e envia o pacote
  LoRa.endPacket();
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
            String body = "leit1=" + stationCode + "&leit2=" + t + "&leit3=" + h;
          
                  sendLoRaPacket(body);                
                }            
       t_ant = t;
       h_ant = h;
      }
      
      
