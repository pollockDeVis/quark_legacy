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
    
      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("Cucian Hebat@unifi", "dobiqueen");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.on("transactions", event);
    webSocket.begin("dobiqueen.digitalforest.io", 3000);
    // use HTTP Basic Authorization this is optional remove if not needed
   // webSocket.setAuthorization("username", "password");
}

void loop() {

  
    if(Serial.available()>0)
    {
      char incomingByte = Serial.read();
      Serial.print(incomingByte);
    }

                  
    webSocket.loop();

    if (websocketReceivedEvent == true)
    {
      websocketReceivedEvent = false;
       
     
      //Serial.print("TOKENS  :");
      //Serial.println(rxTokens);
      tokenizer(rxTokens);
    
    }
}
