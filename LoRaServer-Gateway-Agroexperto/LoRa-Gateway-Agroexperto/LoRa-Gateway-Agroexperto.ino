
//GATEWAY com DDNS

#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
#define RELAY 13

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID     "xxxxxxxxxxx"
#define  PASSWORD "xxxxxxxxxxx"
const char* ssid = SSID;
const char* password = PASSWORD;

//string que recebe o pacote lora
String loraPacket = "";
//string que recebe o pacote wifi
String wifiPacket = "";

//url do servidor para enviar dados
const char* serverName = "https://agroexperto.xxxxxxxxxxxxxxxxx";

//url do servidor para receber comandos
const char* cmdName = "https://agroexperto.xxxxxxxxxx";

//identificação da estação (código AgroexPerto)
String stationCode = "AgroexPerto-Gate-1";

unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

//Instancia cliente wifi
WiFiClient client;

// Porta do server que vc vai utilizar para conectar pelo aplicativo
const int port = 80; 
// Objeto WiFi Server, o ESP será o servidor
WiFiServer server(port);

// Vetor com os clientes que se conectarão no ESP
//std::vector<WiFiClient> clients;

//Tarefas para verificar novos clientes e mensagens enviadas por estes
Scheduler scheduler;
//void taskNewClients();
void taskHandleClient();
////Tarefa para verificar se uma nova conexão feita por aplicativo está sendo feita
//Task t1(100, TASK_FOREVER, &taskNewClients, &scheduler, true);
////Tarefa para verificar se há novas mensagens vindas de aplicativo
Task t2(100, TASK_FOREVER, &taskHandleClient, &scheduler, true);

//Id e estados deste esp (altere para cada esp)
String ID = "GATEWAY1";
String ID_ON = ID + " ON";
String ID_OFF = ID + " OFF";


//Variável para guardar o valor do estado atual do relê 
String currentState = ID_OFF;

void setup() {
      //Coloca tudo em maiúsculo
      ID_ON.toUpperCase();
      ID_OFF.toUpperCase();    
      Heltec.begin(true /*Ativa o display*/, true /*Ativa lora*/, true /*Ativa informações pela serial*/, true /*Ativa PABOOST*/, BAND /*frequência*/);     
      //Inicializa o display
      setupDisplay();    
      //Ativa o recebimento de pacotes lora
      LoRa.receive();
      //Se conecta à rede WiFi
      setupWiFi();
    
      //Inicializa o server ao qual vc vai se conectar utilizando o ddns
      //server.begin(port);
      
      //Inicializa o agendador de tarefas
      //scheduler.startNow();

      
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
      WiFi.begin(SSID, PASSWORD);    
        //Enquanto o ESP não se conectar à rede
        while (WiFi.status() != WL_CONNECTED){
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

void gatewayDisplay() {
      //Limpa o display
      Heltec.display->clear();
      //Exibe o estado atual do relê
      Heltec.display->drawString(0, 0, currentState);
      //Exibe o ip deste esp para ser utilizado no aplicativo
      Heltec.display->drawString(0, 15, "GATEWAY");
      Heltec.display->display();
    }    

void loop() {
      //Faz a leitura do pacote Lora
      loraPacket = readLoRaPacket(); //retorna a string "loraPacket"
      //Se uma mensagem lora chegou
      if(!loraPacket.equals("")) {
          // enviamos a mensagem por wifi para a rede        
          sendWiFiPacket(loraPacket);
          gatewayDisplay();
          }  
      //recebe comendos pelo wifi
      
      //Executa as tarefas que foram adicionadas ao scheduler
      scheduler.execute();
   }

//Faz a leitura de um pacote (se chegou algum)
String readLoRaPacket() {
        loraPacket = "";
        //Verifica o tamanho do pacote
        int packetSize = LoRa.parsePacket();
        //Lê cada caractere e concatena na string 
        for (int i = 0; i < packetSize; i++) { 
          loraPacket += (char) LoRa.read(); 
          }
        return loraPacket;
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
        }


// Função que verifica se o app enviou um comando
void taskHandleClient(){
      // String que receberá o comando vindo do aplicativo
      String cmd;          
        if(client.available()){
          // Recebemos a String até o '\n'
          cmd = client.readStringUntil('\n');
          // Verificamos o comando, enviando por parâmetro a String cmd
          handleCommand(cmd);
          Serial.println(cmd);        
        }          
      }

// Função que verifica o comando vindo do app
void handleCommand(String cmd){
        // Se a String estiver vazia não precisamos fazer nada
        if (cmd.equals(""))
          return;      
        //Coloca todos os caracteres em maiúsculo
        cmd.toUpperCase();      
        // Exibimos o comando recebido no monitor serial
        Serial.println("Received from app: " + cmd);
          //Envia o comando para os outros esp através de um pacote LoRa
          sendLoRaPacket(cmd);
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
