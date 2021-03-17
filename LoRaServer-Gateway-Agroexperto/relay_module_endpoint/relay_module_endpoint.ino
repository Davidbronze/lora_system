/*
 *  AGROEXPERTO - RECEIVER - RELAY MODULE
 */

#include <ESP8266WiFi.h>

const char* ssid = "teste1";
const char* password = "12345678";
WiFiServer server (80);
IPAddress IP(192, 168, 4, 151);
IPAddress subnet ( 255, 255, 255, 0 );

  //Hex command to send to serial for close relay 1
  byte re1ON[]  = {0xA0, 0x01, 0x00, 0xA1};

  //Hex command to send to serial for open relay 1
  byte re1OFF[] = {0xA0, 0x01, 0x01, 0xA2};

  //Hex command to send to serial for close relay 2
  byte re2ON[]  = {0xA0, 0x02, 0x00, 0xA2};

  //Hex command to send to serial for open relay 2
  byte re2OFF[] = {0xA0, 0x02, 0x01, 0xA3};

  //Hex command to send to serial for close relay 3
  byte re3ON[]  = {0xA0, 0x03, 0x00, 0xA3};

  //Hex command to send to serial for open relay 3
  byte re3OFF[] = {0xA0, 0x03, 0x01, 0xA4};

  //Hex command to send to serial for close relay 4
  byte re4ON[]  = {0xA0, 0x04, 0x00, 0xA4};

  //Hex command to send to serial for open relay 4
  byte re4OFF[] = {0xA0, 0x04, 0x01, 0xA5};

void setup ()
{
  delay (10);
  Serial.begin (115200);
  Serial.println();
  Serial.print("Setting soft-AP configuration ... ");
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!"); // (”ssid”, “password”) do AP   
  server.begin();
  Serial.print("Server em: ");
  Serial.println(WiFi.softAPIP());
  delay(50);

}

void loop() {
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if ( ! client ) {
    return;
  }

  Serial.println("Novo cliente conectou");  
 
  // Read the first line of the request
  String req = client.readStringUntil ('\r');

  Serial.print("Requisicao: ");
  Serial.println(req);
  
  //alterar esta parte do código para inserir os comandos dos outros relays

  //int stat1;
  //int stat2;
  //int stat3;
  //int stat4;
  
  if (req.indexOf ("relay1On") != -1)      //relay 1 on
  {
    Serial.write (re1ON, sizeof(re1ON));
    Serial.println("relay 1 on");
   // stat1 = 1; // if you want feedback see below
  }
  else if (req.indexOf ("relay1Off") != -1) //relay 1 off
  {
      Serial.write (re1OFF, sizeof(re1OFF));
      Serial.println("relay 1 off");
     // stat1 = 0; // if you want feedback
  }
   else if (req.indexOf ("relay2On") != -1) //relay 2 on
  {
      Serial.write (re2ON, sizeof(re2ON));
      Serial.println("relay 2 on");
     // stat2 = 1; // if you want feedback
  }
   else if (req.indexOf ("relay2Off") != -1)  //relay 2 off
  {
      Serial.write (re2OFF, sizeof(re2OFF));
      Serial.println("relay 2 off");
     // stat3 = 0; // if you want feedback
  }
  else if (req.indexOf ("relay3On") != -1) //relay 3 on
  {
      Serial.write (re3ON, sizeof(re3ON));
      Serial.println("relay 3 on");
     // stat3 = 1; // if you want feedback
  }
   else if (req.indexOf ("relay3Off") != -1) //relay 3 off
  {
      Serial.write (re3OFF, sizeof(re3OFF));
      Serial.println("relay 3 off");
    //  stat3 = 0; // if you want feedback
  }
  else if (req.indexOf ("relay4On") != -1) //relay 4 on
  {
      Serial.write (re4ON, sizeof(re4ON));
      Serial.println("relay 4 on");
    //  stat4 = 1; // if you want feedback
  }
   else if (req.indexOf ("relay4Off") != -1) //relay 4 off
  {
      Serial.write (re4OFF, sizeof(re4OFF));
      Serial.println("relay 4 off");
     // stat4 = 0; // if you want feedback
  }


//  s += ((stat1)?"on ":" off ") + ((stat2)?"on ":"off ") + ((stat3)?"on ":"off ") + ((stat4)?"on ":"off ");
//  s += "</html>\n";
//  // Send the response to the client
//  client.print (s);
  delay (1000);

//Fecha a conexao
client.stop();
Serial.println("Cliente desconectado");

}
