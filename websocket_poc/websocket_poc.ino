/*
 * WebSocketClientSocketIO.ino
 *
 *  Created on: 06.06.2016
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;


#define USE_SERIAL Serial

#define MESSAGE_INTERVAL 30000
#define HEARTBEAT_INTERVAL 25000
#define DEBUG_MESSAGES
uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

char* jsonString;
bool jsonReceived = false;
StaticJsonBuffer<200> jsonBuffer;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


    switch(type) {
        case WStype_DISCONNECTED:
        
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
                isConnected = true;

          // send message to server when Connected
                // socket.io upgrade confirmation message (required)
        webSocket.sendTXT("5");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
           jsonString = (char*)payload;
           jsonReceived = true;
            //parse payload/save it to a variable

      // send message to server
      // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
    }

}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }
  

    WiFiMulti.addAP("Einstein", "unmc94ESS");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.print("*");
    }

    webSocket.beginSocketIO("dobiqueen.digitalforest.io", 3000);
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);

}

void loop() {
    webSocket.loop();
if(jsonReceived){

Serial.println("JSON OBJECT");
 JsonObject& root = jsonBuffer.parseObject(jsonString);
 
if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  int tokens = root["tokens"];

  Serial.print("Received tokens");
  Serial.println(tokens);
  jsonReceived = false;
}
//    if(isConnected) {
//
//        uint64_t now = millis();
//
//        if(now - messageTimestamp > MESSAGE_INTERVAL) {
//            messageTimestamp = now;
//            // example socket.io message with type "messageType" and JSON payload
//            webSocket.sendTXT("42[\"messageType\",{\"greeting\":\"hello\"}]");
//        }
//        if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
//            heartbeatTimestamp = now;
//            // socket.io heartbeat message
//            webSocket.sendTXT("2");
//        }
//    }
}
