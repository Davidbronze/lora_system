//SERVER (Gateway remoto)

#include <heltec.h>
#include <TaskScheduler.h>
#include <vector>
#include <WiFi.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
#define RELAY 13

//SSID e senha do roteador ao qual o gateway vai conectar
 const char* ssid = "xpertinho";
  const char* password = "xpert-secret";
  IPAddress staticIP(192,168,1, 200); IP do STA
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

#define DHTTYPE DHT22  //Define o tipo de sensor DHT
#define DHTPIN 5   //P5 D1 Pino de ligacao do DHT22
#define PIN_VL 4  // P4  D2  Leitura do  Pluviômetro
#define PL 12     //P12  D6  RST Pluviometro
//observar que o LCD está nos pinos P2 e P14

//Instancia o sensor
DHT dht(DHTPIN, DHTTYPE);

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

// Porta do server que vc vai utilizar para conectar no endpoint
const int port = 80; 

//Instancia cliente wifi
WiFiClient client;

//Tarefas para verificar novos clientes e mensagens enviadas por estes
Scheduler scheduler;
void taskNewClients();
void taskHandleClients();
//Tarefa para verificar se uma nova conexão feita por aplicativo está sendo feita
Task t1(100, TASK_FOREVER, &taskNewClients, &scheduler, true);
//Tarefa para verificar se há novas mensagens vindas de aplicativo
Task t2(100, TASK_FOREVER, &taskHandleClients, &scheduler, true);

//Id e estados deste esp (altere para cada esp)
String ID = "ENDPOINT1";
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
  LoRa.send();

  //Se conecta à rede WiFi
  setupWiFi();
  

  //Modifica o estado do relê para desligado
  verifyAndSetRelayState(ID_OFF);

  //Inicializa o agendador de tarefas
  scheduler.startNow();
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

void loop() {
  //Faz a leitura do pacote
      String packet = readLoRaPacket();
      //Se uma mensagem chegou
      if(!packet.equals("")) {
      handleCommand(packet);
      }
      //Executa as tarefas que foram adicionadas ao scheduler
      scheduler.execute();
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

void readWeather(){
  
