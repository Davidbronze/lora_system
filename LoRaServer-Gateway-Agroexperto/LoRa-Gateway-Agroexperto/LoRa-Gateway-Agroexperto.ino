

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

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID    
const char* ssid = SSID;
const char* password = PASSWORD;
IPAddress staticIP(192,168,5, 199); //IP do GATEWAY
IPAddress gateway ( 192, 168, 5, 1);
IPAddress subnet ( 255, 255, 255, 0 );

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"xEQeg3v65yBl98Fvh1wSTmDeXvYt\n" \
"-----END CERTIFICATE-----\n";

const char* hostAgro = "agroexperto.com.br";

//string que recebe o pacote lora
String loraPacket = "";

//url do servidor para enviar dados
const char* serverName = "https://agroexperto.com.br/databank/inseredados.php";

//identificação da estação (código AgroexPerto)
String stationCode = "xx-Gate-1";

unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura
unsigned long lastTimeCmd = 0;

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

// Porta do server que vc vai utilizar para conectar pelo aplicativo
const int port = 80; 
// Objeto WiFi Server, o ESP será o servidor
WiFiServer server(port);

WiFiMulti wiFiMulti;

WiFiClientSecure client;

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
String ID_ON = ID + " ON";
String ID_OFF = ID + " OFF";
String appCmd = "";
bool ledStatus = false;
//Variável para guardar o valor do estado atual do relê 
String currentState = ID_OFF;
String packSize = "--";
String rssi = "RSSI --";

void setup() {
      //Coloca tudo em maiúsculo
      ID_ON.toUpperCase();
      ID_OFF.toUpperCase();    
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
      WiFi.config (staticIP, gateway, subnet);
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
      Heltec.display->drawString(0, 0, currentState);
      //Exibe o ip deste esp para ser utilizado no aplicativo
      Heltec.display->setFont(ArialMT_Plain_10);
      Heltec.display->drawString(0, 25, state);
      Heltec.display->display();
    }

void gatewayDisplay(String pct) {
      //Limpa o display
      Heltec.display->clear();
      //Exibe o estado atual do relê
      Heltec.display->drawString(0, 0, currentState);
      Heltec.display->drawString(0, 25, "Msg recebida: ");
      Heltec.display->drawString(0, 40, pct);
      Heltec.display->display();
    }    

void loop() {
       //Executa as tarefas que foram adicionadas ao scheduler
              
      scheduler.execute();

      //enviamos a mensagem por wifi para a rede
      if (!loraPacket.equals("")){
      sendWiFiPacket(loraPacket);
      } 
     }

void onReceive(int packetSize)//LoRa receiver interrupt service
    {
      //if (packetSize == 0) return;
      loraPacket = "";
        packSize = String(packetSize,DEC);
        while (LoRa.available()) {
        loraPacket += (char) LoRa.read();
        }
        rssi = "RSSI: " + String(LoRa.packetRssi(), DEC);
        Serial.println("pacote lora recebido, nivel " + rssi);
        Serial.println("pacote: " + loraPacket);         
    }


void sendWiFiPacket(String str){
      if(wiFiMulti.run() == WL_CONNECTED){
      //if (WiFi.status() == WL_CONNECTED){
        client.setCACert(rootCACertificate);
        client.connect(serverName, 443);       
        delay(500);
        if (client.connected() == false){
            Serial.println("Connection failed!");}
        else {
            HTTPClient https; //cria instância do cliente http
            // Inicia o protocolo http com o cliente wifi e a url ou IP do servidor
            https.begin(client, serverName);
            //https.begin(client, serverName);
            // Specify content-type header
            https.addHeader("Host", "agroexperto.com.br");
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");      
            // Send HTTP POST request
            int httpCode = https.POST(str);
            delay(2000);
                if (httpCode < 0) {
                  Serial.println("erro na requisição");
                  Serial.println(https.errorToString(httpCode));
                  Serial.println(https.getString());
                  //delay(60000);                      
                  }
                  else {
                  Serial.println("Conectado");
                  Serial.println(https.getString());
                  }        
            // Free resources
            https.end();
            client.stop();            
            }
          }              
         else {
          Serial.println("WiFi Disconnected");
       }         
       loraPacket = "";
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
