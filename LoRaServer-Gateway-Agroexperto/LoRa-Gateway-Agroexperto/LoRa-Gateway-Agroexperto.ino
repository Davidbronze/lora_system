

//GATEWAY com DDNS - Gateway central


#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>
#include <WiFiMulti.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
//#define RELAY 13

//Colar abaixo as credenciais

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID     "xxxxxxxxxxxxx"
#define  PASSWORD "xxxxxxxxxxxxxx"
const char* ssid = SSID;
const char* password = PASSWORD;
IPAddress staticIP(192, 168, 15, 199); //IP do GATEWAY
IPAddress gateway ( 192, 168, 15, 1);
IPAddress subnet ( 255, 255, 255, 0 );
IPAddress dnsA ( 8, 8, 8, 8);

//url do servidor para enviar dados
const char* serverName = "https://xxxxxxxxx.com.br/xxxxxxxxx.php";

//identificação da estação (código AgroexPerto)
String stationCode = "xx-Gate-1";

const char* hostAgro = "zzzzzzzzz.com.br/xxxxxxxxxxxxxxx.php";

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxx\
"-----END CERTIFICATE-----\n";



unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura
unsigned long lastTimeCmd = 0;

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

// Porta do server que vc vai utilizar para conectar pelo aplicativo
const int port = 443; 
// Objeto WiFi Server, o ESP será o servidor
WiFiServer server(port);

WiFiMulti wiFiMulti;

// Vetor com os clientes que se conectarão no ESP
//std::vector<WiFiClient> clients;

//Tarefas para verificar novos clientes e mensagens enviadas por estes
Scheduler scheduler;
//void taskNewClients();
void taskGetCommand();
////Tarefa para verificar se uma nova conexão feita por aplicativo está sendo feita
//Task t1(100, TASK_FOREVER, &taskNewClients, &scheduler, true);
////Tarefa para verificar se há novas mensagens vindas de aplicativo
Task t1(100, TASK_FOREVER, &taskGetCommand, &scheduler, true);

//Id e estados deste esp (altere para cada esp)
String ID = "GATEWAY1";
String appCmd = "";
bool ledStatus = false;

String packSize = "--";
String rssi = "RSSI --";
//string que recebe o pacote lora
String loraPacket;
bool flag1 = 0;


void setup() {
      //Coloca tudo em maiúsculo
      loraPacket.reserve(50); 
      Heltec.begin(true /*Ativa o display*/, true /*Ativa lora*/, true /*Ativa informações pela serial*/, true /*Ativa PABOOST*/, BAND /*frequência*/);     
      //Inicializa o display
      setupDisplay();    
      //Ativa o recebimento de pacotes lora
      
      //Se conecta à rede WiFi
      setupWiFi();

      LoRa.onReceive(onReceive);
      LoRa.receive();
    
      //Inicializa o server ao qual vc vai se conectar utilizando o ddns
      server.begin();
      
      //Inicializa o agendador de tarefas
      scheduler.startNow();

      Serial.print("core principal= ");
      Serial.println(xPortGetCoreID());

      Serial.println("Setup finalizado");
      delay(5000);
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
      Heltec.display->drawString(0, 25, "Display OK");
      Heltec.display->display();
    }

void setupWiFi() {
      Serial.print("Conectando");
      //Faz o ESP se conectar à rede WiFi
      WiFi.setAutoConnect(true);
      WiFi.config (staticIP, gateway, subnet, dnsA);
      wiFiMulti.addAP(ssid, password);
      //WiFi.begin(ssid, password);    
        //Enquanto o ESP não se conectar à rede
        byte count = 0;
        //while (WiFi.status() != WL_CONNECTED && count < 50){
        while (wiFiMulti.run() != WL_CONNECTED && count < 50){
          count ++; //fazemos "count" tentativas
              //Esperamos 100 milisegundos
              delay(100);
              Serial.print(".");
            }
      if (wiFiMulti.run() == WL_CONNECTED){
      //if (WiFi.status() == WL_CONNECTED){ 
          //Se chegou aqui é porque conectou à rede, então mostramos no monitor serial para termos um feedback
          Serial.println("");
          Serial.println("Conectou");    
          //Objeto que vamos utilizar para guardar o ip recebido
          myIP = WiFi.localIP();
          //Mostra o ip no monitor serial
          Serial.println(myIP);    
          //Atualiza o display para exibir o ip
          refreshDisplay(myIP.toString());
      }
      else {
          Heltec.display -> clear();
          Heltec.display -> drawString(0, 0, "Connecting...");
          Heltec.display -> drawString(0, 25, "...Failed");
          Heltec.display -> display();
          delay(5000);
          refreshDisplay("Not connected to wifi");
      }
    }

void refreshDisplay(String state) {
      //Limpa o display
      Heltec.display->clear();
      Heltec.display->setFont(ArialMT_Plain_16);
      //Exibe o estado atual do relê
      Heltec.display->drawString(0, 0, ID);
      //Exibe o ip deste esp para ser utilizado no aplicativo
      Heltec.display->setFont(ArialMT_Plain_10);
      Heltec.display->drawString(0, 25, state);
      Heltec.display->display();
    }

void gatewayDisplay(String pct) {
      //Limpa o display
      Heltec.display->clear();
      //Exibe o estado atual do relê
      Heltec.display->drawString(0, 0, ID);
      Heltec.display->drawString(0, 25, "Msg recebida: ");
      Heltec.display->drawString(0, 40, pct);
      Heltec.display->display();
    }    

void loop() {
       //Executa as tarefas que foram adicionadas ao scheduler              
      scheduler.execute();

      //enviamos a mensagem por wifi para a rede
      if (flag1 == 1){
        xTaskCreatePinnedToCore(
                    sendWiFiPacket,            /* função que implementa a tarefa */
                    "sendWiFiPacket",          /* nome da tarefa */
                    20000,                     /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    (void *) &loraPacket,   /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,                          /* prioridade da tarefa (0 a N) */
                    NULL,                       /* referência para a tarefa (pode ser NULL) */
                    0);                         /* Núcleo que executará a tarefa */
            flag1 = 0;
            }
             
     }

void onReceive(int packetSize)//LoRa receiver interrupt service
    {
      //if (packetSize == 0) return;
      //rssi = "RSSI: " + String(LoRa.packetRssi(), DEC);
        packSize = String(packetSize,DEC);
        while (LoRa.available()) {
        loraPacket += (char) LoRa.read();
        }
        flag1 = 1;       
    }


void sendWiFiPacket(void *parameter){
      if(wiFiMulti.run() == WL_CONNECTED){
           WiFiClientSecure client;
           client.setCACert(rootCACertificate);
           client.connect(serverName, 443);
           uint8_t indicator = client.connected();
           Serial.print("cliente está conectado? ");
           Serial.println(indicator);
          {     
            HTTPClient httpsAgro; //cria instância do cliente http
            // Inicia o protocolo http com o cliente wifi e a url ou IP do servidor
            Serial.print("begin = ");
            //Serial.println(initialization);
            httpsAgro.begin(serverName, rootCACertificate);
            // Specify content-type header            
            httpsAgro.addHeader("Content-Type", "application/x-www-form-urlencoded");
            // Send HTTP POST request
            Serial.println("pacote a ser enviado: " + loraPacket);
            int httpCode = httpsAgro.POST(loraPacket);
            delay(100);
                if (httpCode < 0) {
                  Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
                  Serial.println(httpsAgro.errorToString(httpCode));
                  //delay(60000);                      
                  }
                else {
                  Serial.print("servidor responde: ");
                  Serial.println(httpsAgro.getString());
                  }        
            // Free resources
            httpsAgro.end();
          }
            client.stop();            
            
          }              
      else {
          Serial.println("WiFi Disconnected");
       }
                
       loraPacket = "";
       Serial.print("loraPacket = ");
       Serial.println(loraPacket);
       vTaskDelete(NULL);         
   }


// Função que verifica se o app enviou um comando
void taskGetCommand(){
        if(millis()-lastTimeCmd >=13000){
          ledStatus == true ? ledStatus = false : ledStatus = true;
          digitalWrite(25, LOW);
          lastTimeCmd = millis(); 
          ledStatus == true ? appCmd = "REMOTE1relay2On" : appCmd = "REMOTE1relay2Off";
              //Instancia cliente wifi
              //WiFiClient wifiClient = server.available();                
              // if(wifiClient.available()){
                // Recebemos a String até o '\n'
              //  appCmd = wifiClient.readStringUntil('\n');
          // Verificamos o comando, enviando por parâmetro a String appCmd
          handleCommand(appCmd);}
      }
//
// Função que verifica o comando vindo do app
void handleCommand(String cmd){
        // Se a String estiver vazia não precisamos fazer nada
        if (cmd.equals(""))
          return;      
        //Coloca todos os caracteres em maiúsculo
       // cmd.toUpperCase();      
        // Exibimos o comando recebido no monitor serial
        Serial.println("Received from app: " + cmd);
        //refreshDisplay(cmd);
          //Envia o comando para os REMOTES através de um pacote LoRa
          digitalWrite(25, HIGH);
          sendLoRaPacket(cmd);
          LoRa.receive();
                          
        }

        //Envia um pacote LoRa
void sendLoRaPacket(String str) {
        //Inicializa o pacote
        LoRa.beginPacket();
        //Coloca a string no pacote
        LoRa.print(str);
        //Finaliza e envia o pacote
        LoRa.endPacket();
        refreshDisplay(str);
        digitalWrite(25, LOW);
      }
