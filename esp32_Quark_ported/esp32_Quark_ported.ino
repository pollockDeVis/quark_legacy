#include <SocketIoClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Ticker.h>
#include "tokenizer.h"



#define SERIALDEBUG 0 //WEBSOCKETS DEBUG. CHANGE VALUE TO 1 TO TURN IN ON
#define CASHTRANSACTION 0
#define USE_SERIAL Serial
#define LED_PIN 17
Ticker ticker; //Ticker for Interrupt

//QUARK PARAMETERS
const char* TERMINAL    PROGMEM = "DB000006";
const int   TERMINAL_ID PROGMEM = 6;
const char* TERMINAL_PASSWORD  PROGMEM = "123456";
const char* FIRMWARE_VERSION = "1.0.4"; 
const char* HARDWARE_VERSION = "1.0.1";
//SERVER DETAILS
const char* host  PROGMEM = "dobiqueen.digitalforest.io";
//WIFI CREDENTIALS
const char* ssid PROGMEM = "SciFi_2.4Ghz";
const char* password PROGMEM = "df2019ROUTER";
const unsigned long wifi_timeout PROGMEM = 10000; // 10 seconds waiting for connecting to wifi
const unsigned long wifi_reconnect_time PROGMEM = 120000; // 2 min retrying
unsigned long wifi_last_connected_time = millis();
bool WIFI_ACTIVE = true;
//WEBSOCKET PARAMETERS
const int webSocketPort = 2052;
bool websocketReceivedEvent = false;
bool successfulTxn = false;
int rxTokens;
int WS_txID;
int WS_terminalID;

SocketIoClient webSocket;

char incomingByte;
bool incomingFlag = false;
void connectEvent(const char * payload, size_t length)
{
#if SERIALDEBUG
  Serial.println("WEBSOCKETS CONNECTING");
#endif

  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["username"] = TERMINAL;
  root["password"] = TERMINAL_PASSWORD;

  int JSONlength = root.measureLength();
  char buffer[JSONlength + 1];
  root.printTo(buffer, sizeof(buffer));
  webSocket.emit("authentication", buffer);

}

void event(const char * payload, size_t length) 
{
  DynamicJsonBuffer  jsonBuffer(200);
  websocketReceivedEvent = true;
#if SERIALDEBUG
  USE_SERIAL.printf("Size of  buff: %d\n", length);
  USE_SERIAL.println("INCOMING PAYLOAD");
  USE_SERIAL.println(payload);
#endif
  //Websocket transaction payload example
  //{"id":4,"customer_id":1,"terminal_id":1,"tokens":4,"status":4,"transaction_at":null,"created_at":"2019-05-07 17:26:39","updated_at":"2019-05-07 17:26:39","deleted_at":null}


  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success())
  {
#if SERIALDEBUG
    Serial.println("parseObject() failed");
#endif
    return;
  }
  rxTokens = root["tokens"];
  WS_txID = root["id"];
  WS_terminalID = root["terminal_id"];

#if SERIALDEBUG
  Serial.print("Transaction ID ");
  Serial.println(WS_txID);
  Serial.print("Terminal ID ");
  Serial.println(WS_terminalID);
#endif


}
void setup() {
  // put your setup code here, to run once:
USE_SERIAL.begin(9600);
USE_SERIAL.setDebugOutput(false);
pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, LOW);
/*******WIFI SETUP********************************************************************/  
  connectWifi();
/*******************************WEBSOCKET SETUP**************************************************************/
  webSocket.on("connect", connectEvent);
  webSocket.on("transaction", event);
  webSocket.begin(host, webSocketPort);

ticker.attach_ms(500, checkSerialISR); //(time in seconds, isrFunction)
}

void loop() {
  // put your main code here, to run repeatedly:
 WIFI_ACTIVE = wifiStatusCheck();



     if(WIFI_ACTIVE == true)
  {
        
   
      webSocket.loop();
  
  /************CHECK ONLINE TXNS******************************************************************/
    if (websocketReceivedEvent == true)
    {
      websocketReceivedEvent = false;

      #if SERIALDEBUG
        Serial.print("TOKENS  :");
        Serial.println(rxTokens);
      #endif

      //CHECK TERMINAL ID #TODO
      if(WS_terminalID == TERMINAL_ID)
      {
        tokenizer(rxTokens);
      }


    }
  }
  else
  {
    if((millis() - wifi_last_connected_time) > wifi_reconnect_time && incomingFlag == false) // reconnects every 2 minutes || Serial is checked so that it doesn't go into reconnecting the wifi when transaction is taking place
    {
      wifi_last_connected_time = millis(); // Readjusting the timer so that it doesn't keep on reconnecting on every loop after 2 min has elapsed
    #if SERIALDEBUG
      Serial.print("Connection Lost, Reconnecting");
    #endif
      connectWifi();
    }
  }
}

bool wifiStatusCheck()
{
  if( WiFi.status() == WL_CONNECTED)// ESP32 MOD WiFiMulti.run()
  {      
    digitalWrite(LED_PIN, HIGH);   
    return true;
  }
  else
  {    
    return false;
    digitalWrite(LED_PIN, LOW);
  }
}
void connectWifi()
{
    #if SERIALDEBUG
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
  #endif
  for (uint8_t t = 4; t > 0; t--) 
  { digitalWrite(LED_PIN, HIGH);  
    #if SERIALDEBUG
      USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    #endif
    USE_SERIAL.flush();
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
  // ESP32 MOD WiFiMulti.addAP(ssid, password);
  WiFi.begin(ssid, password);
  unsigned long lastTime = millis();
  
  while (wifiStatusCheck() == false && (millis()-lastTime) < wifi_timeout) 
  {digitalWrite(LED_PIN, HIGH);
    delay(100);
    #if SERIALDEBUG
      Serial.println(".");
      Serial.print("Time: ");
      Serial.println(millis()-lastTime);
    #endif
    digitalWrite(LED_PIN, LOW);
  }
  #if SERIALDEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}

void checkSerialISR()
{
  if (Serial.available() > 0)
  {
//     #if SERIALDEBUG
//      Serial.print("Serial Availability: ");
//      Serial.println(Serial.available());
//    #endif
    incomingByte = Serial.read();
    MAIN_SERIAL.println(incomingByte);
  }
}
