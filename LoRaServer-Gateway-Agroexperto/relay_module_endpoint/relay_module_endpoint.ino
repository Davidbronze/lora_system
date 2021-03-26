
/*
 *  AGROEXPERTO - RECEIVER - RELAY MODULE
 */


#include <ESP8266WiFi.h>
#include <espnow.h>

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

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x7C,0x9E,0xBD,0xFC,0x19,0x04}; //mac do remote


// Define variable to store relay state to be sent
String deliverState;

// Define variable to store incoming comand
String incomingCmd;


// Variable to store if sending data was successful
String succesfully;


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
      Serial.print("Last Packet Send Status: ");
      if (sendStatus == 0){
        Serial.println("Delivery success");
      }
      else{
        Serial.println("Delivery fail");
      }
    }

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
      memcpy(&incomingCmd, incomingData, sizeof(incomingCmd));
      Serial.print("Bytes received: ");
      Serial.println(len);
      Serial.println(incomingCmd);
      getRelays(incomingCmd);
    }

void getRelays(String cmd){
  // change relays status
      if (cmd = 1)      //relay 1 on
        {
          Serial.write (re1ON, sizeof(re1ON));
          Serial.println("Relay 1 On ");
             deliverState = "Relay 1 On";
        }
}



//void getRelays(String cmd){
//  // change relays status
//  if (cmd.indexOf ("relay1On") != -1)      //relay 1 on
//        {
//          Serial.write (re1ON, sizeof(re1ON));
//          Serial.println("Relay 1 On ");
//             deliverState = "Relay 1 On";
//        }
//        else if (cmd.indexOf ("relay1Off") != -1) //relay 1 off
//        {
//            Serial.write (re1OFF, sizeof(re1OFF));      
//           // stat1 = 0; // if you want feedback
//        }
//         else if (cmd.indexOf ("relay2On") != -1) //relay 2 on
//        {
//            Serial.write (re2ON, sizeof(re2ON));      
//           // stat2 = 1; // if you want feedback
//        }
//         else if (cmd.indexOf ("relay2Off") != -1)  //relay 2 off
//        {
//            Serial.write (re2OFF, sizeof(re2OFF));      
//           // stat3 = 0; // if you want feedback
//        }
//        else if (cmd.indexOf ("relay3On") != -1) //relay 3 on
//        {
//            Serial.write (re3ON, sizeof(re3ON));      
//           // stat3 = 1; // if you want feedback
//        }
//         else if (cmd.indexOf ("relay3Off") != -1) //relay 3 off
//        {
//            Serial.write (re3OFF, sizeof(re3OFF));
//          //  stat3 = 0; // if you want feedback
//        }
//        else if (cmd.indexOf ("relay4On") != -1) //relay 4 on
//        {
//            Serial.write (re4ON, sizeof(re4ON));
//          //  stat4 = 1; // if you want feedback
//        }
//         else if (cmd.indexOf ("relay4Off") != -1) //relay 4 off
//        {
//            Serial.write (re4OFF, sizeof(re4OFF));
//           // stat4 = 0; // if you want feedback
//        }
//  
//}

//void printIncomingReadings(){
//        // Display Readings in Serial Monitor
//        Serial.println("INCOMING READINGS");
//        Serial.print("Comando: ");
//        Serial.print(incomingCmd);  
//      }
 
void setup() {
        // Init Serial Monitor
        Serial.begin(115200);
        
        // Set device as a Wi-Fi Station
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        Serial.println();
        Serial.print("Modulo relay macaddress: ");
        Serial.println(WiFi.macAddress());

      
        // Init ESP-NOW
        if (esp_now_init() != 0) {
          Serial.println("Error initializing ESP-NOW");
          return;
        }
      
        // Set ESP-NOW Role
        esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
      
        // Once ESPNow is successfully Init, we will register for Send CB to
        // get the status of Trasnmitted packet
        esp_now_register_send_cb(OnDataSent);
        
        // Register peer
        esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 0);
        
        // Register for a callback function that will be called when data is received
        esp_now_register_recv_cb(OnDataRecv);
      }
 
void loop() {  
         
        }
