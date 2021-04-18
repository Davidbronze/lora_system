
//GATEWAY com DDNS - Gateway central

#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
//#define RELAY 13

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID     "xxxxxxxxxxx"
#define  PASSWORD "xxxxxxxxxxx"
const char* ssid = SSID;
const char* password = PASSWORD;
IPAddress staticIP(192,168,5, 199); //IP do GATEWAY
IPAddress gateway ( 192, 168, 5, 1);
IPAddress subnet ( 255, 255, 255, 0 );

//string que recebe o pacote lora
String loraPacket = "";

//url do servidor para enviar dados
const char* serverName = "https://xxxxxxxxxxxxxxxxxxxxxx.php";

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
      WiFi.begin(ssid, password);    
        //Enquanto o ESP não se conectar à rede
        byte count = 0;
        while (WiFi.status() != WL_CONNECTED && count < 50){
          count ++; //fazemos "count" tentativas
              //Esperamos 100 milisegundos
              delay(100);
              Serial.print(".");
            }
      if (WiFi.status() == WL_CONNECTED){ 
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
    }


void sendWiFiPacket(String str){
      if(WiFi.status()== WL_CONNECTED){
            HTTPClient https; //cria instância do cliente http
            // Inicia o protocolo http com o cliente wifi e a url ou IP do servidor
            https.begin(serverName);
            // Specify content-type header
            https.addHeader("Content-Type", "application/x-www-form-urlencoded");      
            // Send HTTP POST request
            int httpCode = https.POST(str);
                if (httpCode < 0) {
                  Serial.println("erro na requisição");
                  Serial.println(https.getString());
                  //delay(60000);
                  return;    
                  }
                  Serial.println("Conectado");
                  Serial.println(https.getString());        
            // Free resources
            https.end();
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
          ledStatus == true ? appCmd = "REMOTE1relay1On" : appCmd = "REMOTE1relay1Off";
              //Instancia cliente wifi
              //WiFiClient client = server.available();                
              // if(client.available()){
                // Recebemos a String até o '\n'
              //  appCmd = client.readStringUntil('\n');
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
