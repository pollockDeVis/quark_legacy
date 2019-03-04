#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
#include <SocketIoClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

bool websocketReceivedEvent = false;
int rxTokens;

#ifndef STASSID
#define STASSID "THREE BROTHERS"
#define STAPSK  "hola1234"
#endif

//#define SERIALDEBUG 1

const char* ssid = STASSID;
const char* password = STAPSK;


//StaticJsonBuffer<1000> jsonBuffer;
DynamicJsonBuffer  jsonBuffer(200);

void event(const char * payload, size_t length) {

  websocketReceivedEvent = true;
 // USE_SERIAL.printf("Size of  buff: %d\n", length);
  
  JsonObject& root = jsonBuffer.parseObject(payload);

      if (!root.success()) 
      {
        Serial.println("parseObject() failed");
        return;
      }
  rxTokens = root["tokens"];
  
}

void setup() {
    USE_SERIAL.begin(9600);

    

    USE_SERIAL.setDebugOutput(false);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();
    
    #if SERIALDEBUG
      Serial.print("Connecting to WiFi");
      Serial.println(ssid);
    #endif
    
      for(uint8_t t = 4; t > 0; t--) {
        #if SERIALDEBUG
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        #endif
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP(ssid, password);

    while(WiFiMulti.run() != WL_CONNECTED) {
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
    
  
    webSocket.on("transactions", event);
    webSocket.begin("dobiqueen.digitalforest.io", 2052);
    // use HTTP Basic Authorization this is optional remove if not needed
   // webSocket.setAuthorization("username", "password");
}

void loop() {

  
    if(Serial.available()>0)
    {
      char incomingByte = Serial.read();
        Serial.println(incomingByte);
      
    }

                  
    webSocket.loop();

    if (websocketReceivedEvent == true)
    {
      websocketReceivedEvent = false;
       
  #if SERIALDEBUG   
      Serial.print("TOKENS  :");
      Serial.println(rxTokens);
  #endif
      tokenizer(rxTokens);

    
    }
}
