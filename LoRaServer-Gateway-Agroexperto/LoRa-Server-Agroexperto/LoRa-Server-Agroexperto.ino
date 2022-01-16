
//SERVER (Gateway remoto)
//placa Heltec LoRa v2
//obs: alterada a linha 316 da lib SimpleDHT para o valor de 10000 e
//     alterada a linha 350 para o valor de 60 (limite entre 0 e 1)

#include <SimpleDHT.h>
#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>
#include <esp_now.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
#define RELAY 13
#define DHTTYPE DHT22
#define PIN_VL 36  // 36  Leitura do  Pluviômetro
#define PL 37     //37  RST Pluviometro
//observar que os pinos 2, 4, 5, 14, 15, 16, 19, 18, 21, 26, 27 não podem ser usados
#define CHANNEL 3
#define PRINTSCANRESULTS 1
#define DELETEBEFOREPAIR 0

//SSID e senha do endpoint
// const char* ssid = "teste1";
//  const char* password = "12345678";
//  IPAddress staticIP(192,168,5, 199); //IP do REMOTE
//  IPAddress gateway ( 192, 168, 1, 1);
//  IPAddress subnet ( 255, 255, 255, 0 );
//  IPAddress server(192, 168, 4, 151);
//  const char* serverNameTemp = "";


  //ESPNOW
  uint8_t macSlaves[] = {0xCA,0x2B,0x96,0x2F,0xD7,0xD2}; //mac do endpoint
  esp_now_peer_info_t slave;

//define e inicializa as variáveis
const int DHTPIN = 23;   //Pino de ligacao do DHT22
float minima = 99;
float maxima = 0;
float temp=0;
float humid=0;
float h_ant = 0;
float t_ant = 0;
int counter = 0;
bool vl;
const long intervDHT = 60000; //Intervalo de tempo entre leituras do DHT
unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura
unsigned long lastDebounceTime = 0;
const long debounceDelay = 200;
bool lastState = 0;
int reedCounter = 0;
float precipitacao = 0.0;
int precip = 0;
// Porta do server que vc vai utilizar para conectar no endpoint
const int port = 80;
//Id e estados deste esp (altere para cada esp)
String ID = "REMOTE1"; // ID desta unidade
String ID_ON = ID + " ON";
String ID_OFF = ID + " OFF";
//identificação da estação (código AgroexPerto)
String stationCode = "SP200"; //id da estação - alterar e conferir depois de depurar o programa
String loraPacket = "";

//Variável para guardar o valor do estado atual do relê 
String currentState = ID_OFF;
String success;
String incomingReadings;

//Instancia o sensor
SimpleDHT22 dht(DHTPIN);

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;


void setup() {
  Serial.println();
        //Coloca tudo em maiúsculo
        ID_ON.toUpperCase();
        ID_OFF.toUpperCase();
      
        //Coloca o pino onde o relê está como saída
        pinMode(RELAY, OUTPUT);
      
        Heltec.begin(true /*Ativa o display*/, true /*Ativa lora*/, true /*Ativa informações pela serial*/, true /*Ativa PABOOST*/, BAND /*frequência*/);
       
        //Inicializa o display
        setupDisplay();
      
        WiFi.mode(WIFI_AP);
        WiFi.softAP("teste2", NULL, CHANNEL, 1, 4);

        Serial.println("macaddress do remote: " + WiFi.macAddress()); //macaddress deste esp: 7C:9E:BD:FC:19:04
              
        ScanForSlave();

        setupESPNOW();
        
        //Ativa o recebimento e envio de pacotes lora
        LoRa.onReceive(onReceive);
        LoRa.receive();

        refreshDisplay("Setup Ok");
        
        delay(5000);
      
        //Inicializa o agendador de tarefas
       // scheduler.startNow();
      }


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t situation) {
        Serial.print("Last Packet Send Status: ");
        Serial.println(situation == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
        
      }

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
        memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
        Serial.print("Bytes received: ");
        Serial.println(len);
        Serial.println(incomingReadings);        
    }


void setupESPNOW(){
        // Init ESP-NOW
        WiFi.disconnect();
        Serial.println("iniciando espnow...");
        delay(5000);
        if (esp_now_init() != ESP_OK) {
          Serial.println("Error initializing ESP-NOW");
          return;
        }
        else if (esp_now_init() == ESP_OK) {
          Serial.println("initialized ESP-NOW");
        } 
 
        Serial.println(sizeof(slave.peer_addr));
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           slave.peer_addr[0], slave.peer_addr[1], slave.peer_addr[2], slave.peer_addr[3], slave.peer_addr[4], slave.peer_addr[5]);
        //Mostramos o Mac Address que foi destino da mensagem
        Serial.print("Trying peer: "); 
        Serial.println(macStr);
        
        // Add peer        
        if (esp_now_add_peer(&slave) != ESP_OK){
          Serial.println("Failed to add peer");
          return;
        }
        Serial.println(esp_now_add_peer(&slave));
        Serial.println("  Peer adicionado: ");       
        
         // Once ESPNow is successfully Init, we will register for Send CB to
        // get the status of Trasnmitted packet
        esp_now_register_send_cb(OnDataSent);
         
        // Register for a callback function that will be called when data is received
        esp_now_register_recv_cb(OnDataRecv);
        delay(5000);
      }


void ScanForSlave() {
          int8_t scanResults = WiFi.scanNetworks();
          // reset on each scan
          bool slaveFound = 0;
          memset(&slave, 0, sizeof(slave));        
          Serial.println("");
          if (scanResults == 0) {
            Serial.println("No WiFi devices in AP Mode found");
          } else {
            Serial.print("Found "); Serial.print(scanResults); Serial.println(" devices ");
            for (int i = 0; i < scanResults; ++i) {
              // Print SSID and RSSI for each device found
              String SSID = WiFi.SSID(i);
              int32_t RSSI = WiFi.RSSI(i);
              String BSSIDstr = WiFi.BSSIDstr(i);        
                if (PRINTSCANRESULTS) {
                  Serial.print(i + 1);
                  Serial.print(": ");
                  Serial.print(SSID);
                  Serial.print(" (");
                  Serial.print(RSSI);
                  Serial.print(")");
                  Serial.println("");
                }
              delay(10);
              // Check if the current device starts with the ssid of the peer
              if (SSID.indexOf("teste1") == 0) {
                // SSID of interest
                Serial.println("Found a Slave.");
                Serial.print(i + 1); Serial.print(": "); Serial.print(SSID);
                Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]");
                Serial.print(" ("); Serial.print(RSSI); Serial.print(")");
                Serial.println("");
                // Get BSSID => Mac Address of the Slave
                int mac[6];
                if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
                  for (int ii = 0; ii < 6; ++ii ) {
                    slave.peer_addr[ii] = (uint8_t) mac[ii];
                  }
                }        
                slave.channel = CHANNEL; // pick a channel
                slave.encrypt = 0; // no encryption
        
                slaveFound = 1;
                // we are planning to have only one slave in this example;
                // Hence, break after we find one, to be a bit efficient
                break;
              }
            }
          }
        
          if (slaveFound) {
            Serial.println("Slave Found, processing..");
            Serial.println(slave.channel);
          } else {
            Serial.println("Slave Not Found, trying again.");
          }        
          // clean up ram
          WiFi.scanDelete();
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


void refreshDisplay(String connection) {
        //Limpa o display
        Heltec.display->clear();
        //Exibe o estado atual do relê
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(0, 0, currentState);
        //Exibe o ip deste esp para ser utilizado no aplicativo
        Heltec.display->drawString(0, 25, "LoRa e ESPNOW");
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(0, 45, connection);
        Heltec.display->display();
      }

void weatherDisplay(int temperatura, int humidade, int max_s, int min_s){
      //Limpa o display
        Heltec.display->clear();
        //Atualiza informacoes da temperatura
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 2, "AgroexPerto " + stationCode);
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(32, 16, String(temperatura));
        //Heltec.display->drawCircle(52, 22, 2);
        Heltec.display->drawString(32, 42, String(humidade));
        //Heltec.display->drawCircle(50, 48, 2);
        //Heltec.display->drawCircle(59, 61, 2);
        //Heltec.display->drawLine(50, 62, 58, 48);
        Heltec.display->setFont(ArialMT_Plain_24);
        Heltec.display->drawString(32, 16, String(temperatura));
        Heltec.display->drawCircle(52, 22, 2);
        Heltec.display->drawString(32, 42, String(humidade));
        Heltec.display->drawCircle(50, 48, 2);
        Heltec.display->drawCircle(59, 61, 2);
        Heltec.display->drawLine(50, 62, 58, 48);
              
        //Atualiza maxima 
        Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(70, 24, "Max");
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(98, 19, String(max_s));
        //Atualiza minima      
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(70, 48, "Min");
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->drawString(98, 43, String(min_s));
        Heltec.display->display();
}


void loop() {
  digitalWrite(25, LOW);
    readPluv();  
     unsigned long currentMillis = millis();       
        if (currentMillis - previousMillis >= intervDHT){  //Verifica se o intervalo já foi atingido
           previousMillis = currentMillis; //Armazena o valor da ultima leitura
          handleWeather();
        }        
     if (!loraPacket.equals("")){
          handleCommand(loraPacket);
         }
      
      //Executa as tarefas que foram adicionadas ao scheduler
      //scheduler.execute();
      
  weatherDisplay(temp, humid, maxima, minima);
      
  }

//Faz a leitura de um pacote (se chegou algum)
void onReceive(int packetSize) {
      loraPacket = ""; 
        while (LoRa.available()){
        loraPacket += (char)LoRa.read(); 
        }
      Serial.println("lora packet =  " + loraPacket);
      Serial.println(packetSize);      
    }

// Função que verifica o comando vindo do gateway
void handleCommand(String cmd){ 
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
              Serial.println("3 Changed Relay status: " + confirmationMessage);
            }
        //Se  é para o endpoint
        else {
          //Envia o comando para o endpoint
          digitalWrite(25, HIGH);
          Serial.println("3 comando vai para o endpoint");
         sendToEnd(cmd);
         cmd = "";
        }
      }
      loraPacket = "";
    }

  //Verifica se estado é valido para este esp e modifica o estado do relê de acordo
//Retorna true se a mensagem for para este esp e false caso contrário
bool verifyDestiny(String state) {
      //Se a mudança de estado pertence ao id vinculado a este esp
      if (state.indexOf("REMOTE1") != -1) {          
              refreshDisplay("recebido");
              Serial.println("2 o comando é para este esp"); 
              return true;                       
               }
       return false;
       Serial.println("2 o comando não é para este esp");
       refreshDisplay("repassado");             
       }


//Função que envia mensagem para o endpoint - module relay
void sendToEnd(String msg) {        
      uint8_t x;
      const uint8_t *peer_addr = slave.peer_addr;
      if (msg.indexOf ("relay1Off") != -1)      //relay 1 off
            {
             x = 10;
            }
      else if (msg.indexOf ("relay1On") != -1)      //relay 1 on
            {
             x = 11;
            }
      else if (msg.indexOf ("relay2Off") != -1)      //relay 2 off
            {
             x = 20;
            }
      else if (msg.indexOf ("relay2On") != -1)      //relay 2 on
            {
             x = 21;
            }
      else if (msg.indexOf ("relay3Off") != -1)      //relay 3 off
            {
             x = 30;
            } 
      else if (msg.indexOf ("relay3On") != -1)      //relay 3 on
            {
             x = 31;
            }
      else if (msg.indexOf ("relay4Off") != -1)      //relay 4 off
            {
             x = 40;
            }
      else if (msg.indexOf ("relay4On") != -1)      //relay 4 on
            {
             x = 41;
            }
         
         Serial.println("5 tentando enviar comando...");
      esp_err_t result = esp_now_send(peer_addr, (uint8_t *) &x, sizeof(x));
        if (result == ESP_OK) {
          Serial.println("6 success sending data" + x);
        }
        else {
          Serial.print("6 error sending:  ");
          Serial.println(result);
        }
    
        refreshDisplay(msg);
        loraPacket = "";
      }
     


//Envia um pacote LoRa
void sendLoRaPacket(String str) {
        //Inicializa o pacote
        LoRa.beginPacket();
        //Coloca a string no pacote
        LoRa.print(str);
        //Finaliza e envia o pacote
        LoRa.endPacket();
        Serial.println("7  " + str);
      }

void handleWeather(){
  //executa a rotina de coleta do DHT                                
              counter = counter + 1; //acrescenta no contador de leituras
              //Le a temperatura e a umidade do ar
              float temperature = 0;
              float humidity = 0;
              int err = SimpleDHTErrSuccess;
                  if ((err = dht.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
                    Serial.print("Read DHT22 failed, err = ");
                    Serial.print(SimpleDHTErrCode(err));
                    Serial.print(",");
                    Serial.println(SimpleDHTErrDuration(err));
                    Serial.println(temperature);
                    return;
                  }

                  //Os valores foram lidos corretamente, então é seguro colocar nas variáveis
                    temp = temperature;
                    humid = humidity;
              
                if (temp > 100) {
                  temp = t_ant;}    //para evitar picos do sensor
                if (humid > 100) {
                  humid = h_ant;}   //para evitar picos do sensor
          
              //Mostra a temperatura e umidade no Serial Monitor
              Serial.print(F("Temperatura: "));
              Serial.print((float)temp);
              Serial.println(F(" °C "));
              Serial.print(F("Umidade Relativa: "));
              Serial.print((float)humid);
              Serial.println(" %");
              Serial.println("=======");
                //Atualiza as variaveis maxima e minima, se necessario
                
                if (temp >= maxima){
                  maxima = temp;}
                if (temp <= minima){
                  minima = temp;}  
              
              //Envia as informacoes para o display
              weatherDisplay(temp, humid, maxima, minima);
              delay(100); 
          //===============================================================
          
                    if(counter >= 2){
                      counter = 0;            
                      
                      //Cria a string com os dados
                      String body = "leit1=" + stationCode + "&leit2=" + temp + "&leit3=" + humid + "&leit4=" + precip;
                      //String body = "LoRa...";
                            sendLoRaPacket(body);
                            LoRa.receive();
                            precipitacao=precip=0;
                            reedCounter = 0;                
                          }            
                 t_ant = temp;
                 h_ant = humid;
                 
                }

void readPluv(){
      // Contagem pulsos pluviometro
        pinMode(PIN_VL, INPUT);
        digitalWrite(PIN_VL, LOW);
        vl = digitalRead(PIN_VL);  // Inicia a leitura do pluviom           
        if ( vl == 1 && lastState == 0 ) {
          lastDebounceTime = millis();
          lastState = 1;
          reedCounter +=1;
          Serial.println(" bascula acionada ");
          }    
        if ((millis() - lastDebounceTime) > debounceDelay) {
          if (lastState == 1 && vl == 0){
             Serial.println("nova leitura");            
            lastState = 0;            
            precipitacao = precipitacao + 0.27;
            precip = round(precipitacao);
            Serial.println(millis() - lastDebounceTime);
            Serial.print ("Ciclos do pluvio = ");
            Serial.println(reedCounter);
            Serial.print("Precipitação = ");
            Serial.print(precipitacao);
            Serial.println(" mm");            
            Serial.println(precip);
          }
      }  
  }
      
