
//GATEWAY com DDNS - Gateway central

#include <HTTP_Method.h>
#include <Uri.h>
#include <WebServer.h>
#include <HttpsOTAUpdate.h>
#include <Update.h>
#include <ESPmDNS.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <heltec.h>
#include <TaskScheduler.h>
#include <WiFi.h>
#include <WiFiMulti.h>

//Frequência
#define BAND 915E6

//Pino onde o relê está
//#define RELAY 13

//Colar abaixo as credenciais

//SSID e senha do roteador ao qual o gateway vai conectar
#define  SSID     "VIVOFIBRA-5F56"

#define  PASSWORD "33d7405f56"

const char* ssid = SSID;
const char* password = PASSWORD;
IPAddress staticIP(192, 168, 15, 111); //IP do GATEWAY
IPAddress gateway ( 192, 168, 15, 1);
IPAddress subnet ( 255, 255, 255, 0 );
IPAddress dnsA ( 8, 8, 8, 8);

//url do servidor para enviar dados
const char* serverName = "https://agroexperto.com.br/databank/inseredados.php";

//identificação da estação (código AgroexPerto)
String stationCode = "xx-Gate-1";
String firmwareVersion = "1.3";

const char* hostAgro = "agroexperto.com.br/databank/inseredados.php";

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"wAahQ8S5gW+dsoLxDB7o4G5SClao2Q==\n" \
"-----END CERTIFICATE-----\n" ;

//============================

//host para OTA

const char* host = "esp32";

//OTA forms

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

unsigned long previousMillis = 0;  //Armazena o valor (tempo) da ultima leitura
unsigned long lastTimeCmd = 0;

//Objeto que vamos utilizar para guardar o ip recebido
IPAddress myIP;

// Porta do server que vc vai utilizar para conectar pelo aplicativo
const int port = 80; 
// Objeto WiFi Server, o ESP será o servidor
WiFiServer server(port);
WebServer webServerOta(80);
WiFiMulti wiFiMulti;

// Vetor com os clientes que se conectarão no ESP

//Id e estados deste esp (altere para cada esp)
String ID = "GATEWAY1-";
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

      updateOTA();
      //Inicializa o server ao qual vc vai se conectar utilizando o ddns
      server.begin();
      
      //Inicializa o agendador de tarefas
      //scheduler.startNow();

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
              Serial.println("");
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
          Serial.println("Conexão wifi falhou");
          refreshDisplay("Not connected to wifi");
      }
    }

void refreshDisplay(String state) {
      //Limpa o display
      Heltec.display->clear();
      Heltec.display->setFont(ArialMT_Plain_16);
      //Exibe o estado atual do relê
      Heltec.display->drawString(0, 0, ID + firmwareVersion);
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

void updateOTA(){
         /*use mdns for host name resolution*/
      if (!MDNS.begin(host)) { //http://esp32.local
        Serial.println("Error setting up MDNS responder!");
        while (1) {
          delay(1000);
        }
      }
      Serial.println("mDNS responder started");
      /*return index page which is stored in serverIndex */
      webServerOta.on("/", HTTP_GET,
                      []() {
                        webServerOta.sendHeader("Connection", "close");
                        webServerOta.send(200, "text/html", loginIndex);
                      });
      webServerOta.on("/command", taskGetCommand);
      webServerOta.on("/serverIndex", HTTP_GET,
                      []() {
                        webServerOta.sendHeader("Connection", "close");
                        webServerOta.send(200, "text/html", serverIndex);
                      });
      /*handling uploading firmware file */
      webServerOta.on("/update", HTTP_POST,
                      []() {
                        webServerOta.sendHeader("Connection", "close");
                        webServerOta.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                        ESP.restart();},
                      []() {
                        HTTPUpload& upload = webServerOta.upload();
                          if (upload.status == UPLOAD_FILE_START) {
                            Serial.printf("Update: %s\n", upload.filename.c_str());
                            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                              Update.printError(Serial);
                            }
                          } else if (upload.status == UPLOAD_FILE_WRITE) {
                            /* flashing firmware to ESP*/
                            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                              Update.printError(Serial);
                            }
                          } else if (upload.status == UPLOAD_FILE_END) {
                            if (Update.end(true)) { //true to set the size to the current progress
                              Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                            } else {
                              Update.printError(Serial);
                            }
                          }
                        });
      webServerOta.begin();
      }


void loop() {
      
       //Executa as tarefas que foram adicionadas ao scheduler              
      //scheduler.execute();

      webServerOta.handleClient();      
      delay(1);
      
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
               appCmd = webServerOta.arg(0);
               Serial.println(appCmd);
               Serial.println("comando recebido");
               webServerOta.send(200,"comando recebido");
          // Verificamos o comando, enviando por parâmetro a String appCmd
          handleCommand(appCmd);
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

      
