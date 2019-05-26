#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
#include "https_requests.h"
#include <SocketIoClient.h>
#include <WiFiClientSecure.h>


/********************CHANGE THE PARAMS BELOW BEFORE INSTALLATION *****************************************************************************/
//QUARK PARAMETERS
const char* TERMINAL    PROGMEM = "DB000001";
const int   TERMINAL_ID PROGMEM = 1;
const char* TERMINAL_PASSWORD  PROGMEM = "123456";
const char* FIRMWARE_VERSION = "1.0.1"; 
const char* HARDWARE_VERSION = "1.0.0";

//WIFI CREDENTIALS
const char* ssid PROGMEM = "THREE BROTHERS";
const char* password PROGMEM = "hola1234";
const unsigned long wifi_timeout PROGMEM = 10000; // 10 seconds waiting for connecting to wifi
const unsigned long wifi_reconnect_time PROGMEM = 120000; // 2 min retrying
unsigned long wifi_last_connected_time = millis();

//DEBUG
#define SERIALDEBUG 0 //WEBSOCKETS DEBUG. CHANGE VALUE TO 1 TO TURN IN ON

/********************CAUTION: DO NOT CHANGE. *****************************************************************************/
//WEBSOCKET PARAMETERS
const int webSocketPort = 2052;
bool websocketReceivedEvent = false;
bool successfulTxn = false;
int rxTokens;
int WS_txID;
int WS_terminalID;

//HTTPS PARAMETERS
bool successfulPOST = true;
bool successfulPATCH = true;
/***************************************************************************************************************/
#define USE_SERIAL Serial


ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

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

void event(const char * payload, size_t length) {
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

void setup() 
{
  USE_SERIAL.begin(9600);
  USE_SERIAL.setDebugOutput(false);
  pinMode(0, INPUT); // D0 Pin
  uint8_t pinVal = digitalRead(0);
  if( pinVal == 0)
  {
    Serial.printf("\r\nTERMINAL: %s\r\nTERMINAL_ID: %d\r\nFIRMWARE_VERSION: %s\r\nHARDWARE_VERSION: %s", 
    TERMINAL, TERMINAL_ID, FIRMWARE_VERSION, HARDWARE_VERSION);
  }

/*******WIFI SETUP********************************************************************/  
  connectWifi();
/*******************************WEBSOCKET SETUP**************************************************************/
  webSocket.on("connect", connectEvent);
  webSocket.on("transaction", event);
  webSocket.begin(host, webSocketPort);

int d[] = {0, 1, 5, 10, 21, 55};
  coin_change_modified(d, 30, 5);

  
} //end setup

void loop() 
{
/***********************CHECK CASH TXNS**************************************************************/
  if (Serial.available() > 0)
  {
     #if SERIALDEBUG
      Serial.print("Serial Availability: ");
      Serial.println(Serial.available());
    #endif
    char incomingByte = Serial.read();
    Serial.println(incomingByte);
    int cashValue = detokenizer(incomingByte);
    if (cashValue > 0) 
    {
      if(wifiStatusCheck() == true) //create a variable offline for TXN
      successfulPOST =  postCashTransaction(cashValue, TERMINAL, TERMINAL_PASSWORD);
      if(successfulPOST == false)
      {
        //Take time stamp and record Cash value to permanent memory for later updates
      }
    }
  }
    
  if(wifiStatusCheck() == false)
  {
    if((millis() - wifi_last_connected_time) > wifi_reconnect_time) // reconnects every 2 minutes || Serial is checked so that it doesn't go into reconnecting the wifi when transaction is taking place
    {
      wifi_last_connected_time = millis(); // Readjusting the timer so that it doesn't keep on reconnecting on every loop after 2 min has elapsed
    #if SERIALDEBUG
      Serial.print("Connection Lost, Reconnecting");
    #endif
      connectWifi();
    }
  }else
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

      if(successfulTxn)
      {
        #if SERIALDEBUG   
          Serial.print("Boolean is True ");
          Serial.println(" Making the flag false");
        #endif
        successfulPATCH = patchConfirmTransaction(TERMINAL, TERMINAL_PASSWORD, WS_txID);
        successfulTxn = false;
        if(successfulPATCH == false)
        {
          //Record WS_TXID for updating later
        }
      }
    }
  }
}// end Loop
bool wifiStatusCheck()
{
  if(WiFiMulti.run() == WL_CONNECTED)
  {  
    //wifi_last_connected_time = millis(); 
    return true;
  }
  else
  {    
    return false;
  }
}
void connectWifi()
{
    #if SERIALDEBUG
    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
    Serial.print("Connecting to WiFi");
    Serial.println(ssid);
  #endif
  for (uint8_t t = 4; t > 0; t--) 
  {
    #if SERIALDEBUG
      USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    #endif
    USE_SERIAL.flush();
    delay(100);
  }
  WiFiMulti.addAP(ssid, password);
  unsigned long lastTime = millis();
  
  while (wifiStatusCheck() == false && (millis()-lastTime) < wifi_timeout) 
  {
    delay(100);
    #if SERIALDEBUG
      Serial.println(".");
      Serial.print("Time: ");
      Serial.println(millis()-lastTime);
    #endif
  }
  #if SERIALDEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
}
