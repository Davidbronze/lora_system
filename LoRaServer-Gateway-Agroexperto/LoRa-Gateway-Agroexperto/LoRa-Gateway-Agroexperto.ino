//GATEWAY com DDNS

#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>

//Frequência
#define BAND 915E6

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID     "VIVOFIBRA-5F56"
#define  PASSWORD "33d7405f56"
const char* ssid = SSID;
const char* password = PASSWORD;

//string que recebe o pacote lora
String loraPacket = "";
//string que recebe o pacote wifi
String wifiPacket = "";

//url do servidor para enviar dados
const char* serverName = "https://agroexperto.com.br/databank/inseredados.php";

//url do servidor para receber comandos
const char* cmdName = "https://agroexperto.com.br/?????.php";

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
std::vector<WiFiClient> clients;

//Tarefas para verificar novos clientes e mensagens enviadas por estes
Scheduler scheduler;
void taskNewClients();
void taskHandleClients();
//Tarefa para verificar se uma nova conexão feita por aplicativo está sendo feita
Task t1(100, TASK_FOREVER, &taskNewClients, &scheduler, true);
//Tarefa para verificar se há novas mensagens vindas de aplicativo
Task t2(100, TASK_FOREVER, &taskHandleClients, &scheduler, true);

//Id e estados deste esp (altere para cada esp)
String ID = "LIGHT1";
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

  //Ativa o recebimento e envio de pacotes
  LoRa.receive();
  LoRa.send();

  //Se conecta à rede WiFi
  setupWiFi();

  //Inicializa o server ao qual vc vai se conectar utilizando o ddns
  server.begin(port);
  
  //Inicializa o agendador de tarefas
  scheduler.startNow();
}

void loop() {
    //Faz a leitura do pacote Lora
    loraPacket = readLoRaPacket();
    //Se uma mensagem lora chegou
    if(!packet.equals("")) {
      // enviamos a mensagem por wifi para a rede        
      sendWiFiPacket();
    }

    //faz a leitura do pacote wifi
    
  
    //Executa as tarefas que foram adicionadas ao scheduler
    scheduler.execute();

    
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
  while (WiFi.status() != WL_CONNECTED)
  {
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

// Task que insere novos clientes conectados no vector
void taskNewClients(){
  // Se existir um novo client atribuimos para a variável
  WiFiClient newClient = server.available(); 
  
  // Se o client for diferente de nulo
  if(newClient) {      
    // Inserimos no vector
    clients.push_back(newClient);
    // Exibimos na serial indicando novo client e a quantidade atual de clients
    Serial.println("New client! size:"+String(clients.size()));
  }
}

// Função que verifica se o app enviou um comando
void taskHandleClients()
{
  // String que receberá o comando vindo do aplicativo
  String cmd;

  // Atualizamos o vector deixando somente os clientes conectados
  refreshConnections();

  // Percorremos o vector
  for(int i=0; i<clients.size(); i++){      
    // Se existir dados a serem lidos
    if(clients[i].available()){
      // Recebemos a String até o '\n'
      cmd = clients[i].readStringUntil('\n');
      // Verificamos o comando, enviando por parâmetro a String cmd
      handleCommand(cmd);        
    }          
  }
}

// Função que verifica se um ou mais clients se desconectaram do server e, se sim, estes clients serão retirados do vector
void refreshConnections(){
  // Flag que indica se pelo menos um client ser desconectou
  bool flag = false;
  
  // Objeto que receberá apenas os clients conectados
  std::vector<WiFiClient> newVector;

  // Percorremos o vector
  for(int i=0; i<clients.size(); i++){
    // Verificamos se o client está desconectado
    if(!clients[i].connected()){
      // Exibimos na serial que um cliente se desconectou e a posição em que ele está no vector (debug)
      Serial.println("Client disconnected! ["+String(i)+"]");
      // Desconectamos o client
      clients[i].stop();
      // Setamos a flag como true indicando que o vector foi alterado
      flag = true;          
    }
    else{
      newVector.push_back(clients[i]); // Se o client está conectado, adicionamos no newVector
    }
  }
  // Se pelo menos um client se desconectou, atribuimos ao vector "clients" os clients de "newVector"
  if(flag) clients = newVector;
}

// Função que verifica o comando vindo do app
void handleCommand(String cmd)
{
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


//Função que envia mensagem para todos os apps conectados
void sendToClients(String msg) {
  for(int i=0; i<clients.size(); i++){
    clients[i].print(msg);
  
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

void sendWiFiPacket(){
  if(WiFi.status()== WL_CONNECTED){    
    
      HTTPClient https; //cria instância do cliente http       
     
      // Inicia o protocolo http com o cliente wifi e a url ou IP do servidor
      https.begin(serverName);

      // Specify content-type header
      https.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      // Send HTTP POST request
      int httpCode = https.POST(loraPacket);
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
