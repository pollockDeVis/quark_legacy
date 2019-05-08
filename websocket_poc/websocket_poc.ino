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
const char* FIRMWARE_VERSION = "1.0.0"; 
const char* HARDWARE_VERSION = "1.0.0";

//WIFI CREDENTIALS
const char* ssid PROGMEM = "THREE BROTHERS";
const char* password PROGMEM = "hola1234";

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
    delay(1000);
  }
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) 
  {
    delay(100);
    #if SERIALDEBUG
      Serial.println(".");
    #endif
  }
  #if SERIALDEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
/*******************************WEBSOCKET SETUP**************************************************************/
  webSocket.on("connect", connectEvent);
  webSocket.on("transaction", event);
  webSocket.begin(host, webSocketPort);
  
} //end setup

void loop() 
{
/***********************CHECK CASH TXNS**************************************************************/
  if (Serial.available() > 0)
  {
    char incomingByte = Serial.read();
    Serial.println(incomingByte);
    int cashValue = detokenizer(incomingByte);
    if (cashValue > 0) 
    {
      postCashTransaction(cashValue, TERMINAL, TERMINAL_PASSWORD);
    }
  }

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
      patchConfirmTransaction(TERMINAL, TERMINAL_PASSWORD, WS_txID);
      successfulTxn = false;
    }
  }
  
}// end Loop
