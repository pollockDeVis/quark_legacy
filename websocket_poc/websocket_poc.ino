#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
#include <SocketIoClient.h>
//#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

bool websocketReceivedEvent = false;
int rxTokens;

#ifndef STASSID
#define STASSID "THREE BROTHERS"
#define STAPSK  "hola1234"
#endif

#define SERIALDEBUG 1

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "dobiqueen.digitalforest.io";
const int webSocketPort = 2052;
const int httpsPort = 443;

const int   client_id = 1;
const char* client_secret = "0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ";

String token_url = "/api/v1/oauth/token";
String cash_url = "/api/v1/cash-transactions";

const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";

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
    
    char* receivedString = getOauthToken();
    String receivedToken = extractOauthToken(receivedString);
    postCashTransaction(receivedToken, 13);
    //Serial.println(receivedToken);
    
    webSocket.on("transactions", event);
    webSocket.begin(host, webSocketPort);
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

char* getOauthToken()
{
  
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
//    return;
  }

  String token_url = "/api/v1/oauth/token";
  Serial.print("requesting URL: ");
  Serial.println(token_url);
  
  DynamicJsonBuffer jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  root["client_id"] = client_id;
  root["client_secret"] = client_secret;
  root["grant_type"] = "client_credentials";
  root["scope"] = "read";
  int length = root.measureLength();
  char buffer[length+1];
  root.printTo(buffer, sizeof(buffer));

  String requestBody = "POST " + token_url + " HTTP/1.1\r\n" + 
                     "Host: " + host + "\r\n" + 
                     "Content-Type: application/json\r\n" + 
                     "Content-Length: " + root.measureLength() +
                     "\r\n\r\n" + buffer;

  client.println(requestBody);
  Serial.println(requestBody);
  Serial.println("request sent");
  String  line = client.readStringUntil('}'); //readString() is significantly slow
//  char* pch;
//  pch = strchr(line, '{');
  line = line + '}';
       Serial.println(line);
       Serial.print("BRACKET STARTS at");
       int headcount = 0;
       int tailcount = 0;
       int arraysize = 0;
       int arraycounter = 0;
       while (line[headcount] != '{')
       {
        headcount++;
        
        if (line[headcount] == '{') break;
       }
       
      
       while (line[tailcount] != '}')
       {
        tailcount++;
        if (line[tailcount] == '}') break;
       }
        Serial.println(headcount);
        Serial.println(tailcount);
        arraysize = tailcount-headcount + 1;
        char tempBufferforJSON[arraysize];
        for (int counter = headcount; counter < tailcount+1; counter++){
          tempBufferforJSON[arraycounter] = line[counter];
          arraycounter++;
        }
        char* pointer;
        pointer = tempBufferforJSON;
        return pointer;
        //Serial.print("The array is ");
        //Serial.println(tempBufferforJSON);
//       while()
}
String extractOauthToken(char* stringobject){
  
  DynamicJsonBuffer  jsonBuffer(1500);
  JsonObject& root = jsonBuffer.parseObject(stringobject);

      if (!root.success()) 
      {
        Serial.println("parseObject() failed");
        //return;
      }
  String access_token = root["access_token"];
  return access_token;
}




void postCashTransaction(String access_token, int token){

  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
//    return;
  }

  
  Serial.print("requesting URL: ");
  Serial.println(cash_url);
  
  DynamicJsonBuffer jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
   root["terminal"] = client_id;
   root["tokens"] = token;
  int length = root.measureLength();
  char buffer[length+1];
  root.printTo(buffer, sizeof(buffer));


  
  String requestBody2 = "POST " + cash_url + " HTTP/1.1\r\n" + 
                     "Host: " + host + "\r\n" + 
                     "Content-Type: application/json\r\n" + 
                     "Content-Length: " + root.measureLength() +
                     "\r\nAuthorization: " + access_token +
                     "\r\n\r\n" + buffer; 

client.println(requestBody2);
Serial.println(requestBody2);
 String  line2 = client.readStringUntil('}'); //readString() is significantly slow
       Serial.println(line2);
}
