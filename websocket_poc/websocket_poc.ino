#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
#include <SocketIoClient.h>
//#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#define USE_SERIAL Serial
//#define SOCKETIOCLIENT_DEBUG
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
const int webSocketPort = 2052; //2052
const int httpsPort = 443;

const int   client_id = 2;
const char* client_secret = "xj5CtbPW7LNXOE5AvwY7BUGgfjJ0HAH9fA2gBYMe";

String token_url = "/api/v1/oauth/token";
String cash_url = "/api/v1/cash-transactions";

const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";
//char json[] =
 //     "{\"username\":\"DB000001\",\"password\":123456}";

//StaticJsonBuffer<1000> jsonBuffer;
DynamicJsonBuffer  jsonBuffer(200);

void connectEvent(const char * payload, size_t length)
{
  //webSocket.webSocketEvent("authentication","{\"username\":\"DB000001\",\"password\":\"123456\"}");
//  webSocket.sendTXT("authentication");
//  webSocket.sendTXT("{\"username\":\"DB000001\",\"password\":\"123456\"}");
  Serial.println("EMITTING WEBSOCKETS");
  Serial.println("SENDING AUTHORIZATION WEBSOCKET");
  webSocket.emit("authentication","{\"username\":\"DB000001\",\"password\":\"123456\"}");
  //webSocket.on("authenticated", event2);
   
}
void event2(const char * payload, size_t length) {
  Serial.println("WEBSOCKET DISCONNECTED >>>>");
  for(int i = 0; i < strlen(payload); i++)
  {
    Serial.print(payload[i]);
  }
}
void event3(const char * payload, size_t length) {
    Serial.println("3 PAYLOAD INCOMING");
  for(int i = 0; i < strlen(payload); i++)
  {
    Serial.print(payload[i]);
  }
}

void event4(const char * payload, size_t length) {
    Serial.println("4 PAYLOAD INCOMING");
  for(int i = 0; i < strlen(payload); i++)
  {
    Serial.print(payload[i]);
  }
}

void event(const char * payload, size_t length) {

  websocketReceivedEvent = true;
  #if SERIALDEBUG
  USE_SERIAL.printf("Size of  buff: %d\n", length);
  Serial.println("PAYLOAD INCOMING");
  for(int i = 0; i < strlen(payload); i++)
  {
    Serial.print(payload[i]);
  }
  
  #endif
  JsonObject& root = jsonBuffer.parseObject(payload);

      if (!root.success()) 
      {
        #if SERIALDEBUG
        Serial.println("parseObject() failed");
        #endif
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
    Serial.print("The size of the string client_secret is ");
    Serial.println(strlen(client_secret));
#if SERIALDEBUG
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
    
     Serial.println("BEGINNING WEBSOCKETS");
    webSocket.begin(host, webSocketPort);
   // webSocket.beginSSL(host, webSocketPort, "/socket.io/?transport=websocket", fingerprint);
    //Serial.println(receivedToken);
    //webSocket.begin(host, webSocketPort);
    Serial.println("Connecting WEBSOCKETS");
    webSocket.on("connect", connectEvent);
     webSocket.on("disconnect", event2);
    //webSocket.emit("connect","{\"username\":\"DB000001\",\"password\":\"123456\"}");
 
     
      Serial.println();

    
      webSocket.on("transaction", event);
    webSocket.on("authenticated", event3);
    webSocket.on("unauthorized", event4);

   // webSocket.emit("authentication","{\"username\":\"DB000001\",\"password\":\"123456\"}");
    
    // use HTTP Basic Authorization this is optional remove if not needed
//char* client_id_char[20];

//webSocket.setAuthorization("DB000001", "123456");


approveTransaction();
}

void loop() {

  
    if(Serial.available()>0)
    {
      char incomingByte = Serial.read();
        Serial.println(incomingByte);
      int cashValue = detokenizer(incomingByte);
      if (cashValue > 0){
        postCashTransaction(cashValue);
      }
    }
//Serial.println("LOOPING");
                  
    webSocket.loop();
//
//    if (websocketReceivedEvent == true)
//    {
//      websocketReceivedEvent = false;
//       
//  #if SERIALDEBUG   
//      Serial.print("TOKENS  :");
//      Serial.println(rxTokens);
//  #endif
//      tokenizer(rxTokens);
//
//    
//    }
}

char* getOauthToken()
{
  
  WiFiClientSecure client;
  #if SERIALDEBUG
  Serial.print("connecting to ");
  Serial.println(host);
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  #endif
  
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    #if SERIALDEBUG
    Serial.println("connection failed");
    #endif
    return "X";
  }

  String token_url = "/api/v1/oauth/token";
  #if SERIALDEBUG
  Serial.print("requesting URL: ");
  Serial.println(token_url);
  #endif 
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
  #if SERIALDEBUG
  Serial.println(requestBody);
  Serial.println("request sent");
  #endif 
  String  line = client.readStringUntil('}'); //readString() is significantly slow
//  char* pch;
//  pch = strchr(line, '{');
  line = line + '}';
  #if SERIALDEBUG
       Serial.println(line);
       Serial.print("BRACKET STARTS at");
       #endif 
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
       #if SERIALDEBUG
        Serial.println(headcount);
        Serial.println(tailcount);
        #endif 
        
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
        #if SERIALDEBUG
        Serial.println("parseObject() failed");
        #endif 
        //return;
      }
  String access_token = root["access_token"];
  return access_token;
}




void postCashTransaction(String access_token, int token){

  WiFiClientSecure client;
  #if SERIALDEBUG
  Serial.print("connecting to ");
  Serial.println(host);
  #endif 
#if SERIALDEBUG
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  #endif 
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    #if SERIALDEBUG
    Serial.println("connection failed");
    #endif 
//    return;
  }

  #if SERIALDEBUG
  Serial.print("requesting URL: ");
  Serial.println(cash_url);
  #endif 
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
#if SERIALDEBUG
Serial.println(requestBody2);
#endif
 String  line2 = client.readStringUntil('}'); //readString() is significantly slow
 #if SERIALDEBUG
       Serial.println(line2);
       #endif
}
int detokenizer(char Cashsignal)
{
  if(Cashsignal == '@')
  {
    return 1;
  }
  else if(Cashsignal == 'B')
  {
    return 5;
  }
  else if(Cashsignal == 'C')
  {
    return 10;
  }
  else if(Cashsignal == 'F')
  {
    return 20;
  }
  else if(Cashsignal == 'D')
  {
    return 50;
  }
  else{
    return 0;
  }
}



void approveTransaction()
{
  
  WiFiClientSecure client;
  #if SERIALDEBUG
  Serial.print("connecting to ");
  Serial.println(host);
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  #endif
  
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    #if SERIALDEBUG
    Serial.println("connection failed");
    #endif
    //return "X";
  }

  String token_url = "/api/v1/transactions/1";
  #if SERIALDEBUG
  Serial.print("requesting URL: ");
  Serial.println(token_url);
  #endif 
  DynamicJsonBuffer jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  root["action"] = "complete";
//  root["client_secret"] = client_secret;
//  root["grant_type"] = "client_credentials";
//  root["scope"] = "read";
  int length = root.measureLength();
  char buffer[length+1];
  root.printTo(buffer, sizeof(buffer));

  String requestBody = "PATCH " + token_url + " HTTP/1.1\r\n" + 
                     "Host: " + host + "\r\n" + 
                     "Content-Type: application/json\r\n" + 
                     "Content-Length: " + root.measureLength() +
                     "\r\nAuthorization: " + "Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjgwYjQ3ZTI1ZTIyNGFmMDhiZTExMTEwYmE3M2ZmZDlmM2ZiYTIyOWYwYmFkMTM0ZjA2NTBkNDg2MGM0ZDNlMGVhYWFmY2JkOWU3MzRkMTExIn0.eyJhdWQiOiIyIiwianRpIjoiODBiNDdlMjVlMjI0YWYwOGJlMTExMTBiYTczZmZkOWYzZmJhMjI5ZjBiYWQxMzRmMDY1MGQ0ODYwYzRkM2UwZWFhYWZjYmQ5ZTczNGQxMTEiLCJpYXQiOjE1NTcxNjEwNTAsIm5iZiI6MTU1NzE2MTA1MCwiZXhwIjoxNTg4NzgzNDUwLCJzdWIiOiIxX3Rlcm1pbmFsIiwic2NvcGVzIjpbIioiXX0.gQijCiSqS1FVRLBHa3KJhqBdxNPWYpxq8THBiKj8zppIDr9CRpvS4p9VeRx4WJUOsIZnHIwhxf2H0dXQWg34cSggdQhv0DGJXbRmbNCW9h4wXFsjb9QEAKeeH_uPbR9KYdMmhaYxlnk9FiSDJnThCJf5DMwNQulcayH5ESCr-AD-Ihyfv6iRTISiYqLvbfysbwth3MJ6twGGSI3ONfdCY7x5vc-YcgwIMMuIwG5LWzzNUjDgUvlNwH-2ai0TkIvURfn8t-5LnDeq-SMf7PlTQfB9iaFGUrss0Qa4Io3_5gFSlvUp98RYOxgydYg0SQoZjfuCI-uxjh8n5ILksx45O6B_bYdKkhwvHcKLqHMwXRHFoMnWo4rblAhEs-ukd4HbN_oeepf4KRwGxBSyE0j4Ez4ALVlvoRx1VGpoFpOACDJBCFVRPHN5ygWFUA4kZNR20nNHR-nQ6JdJxD4UDIMCQoFnXqehtODCCVezvw3zf5AMu1hQHWGDnwUKmlY1On_fy1r_sGmBAjOraZczGpPYCh7H4CGTxYSscJWCZ0g1z2fOIqmFq745ghGRsO_5YtfzFfRqFNJgWtXP2Q833RKfqGMrv8xKqA8Io-gfX6nkZKIx07-fVLXnjZqkCpuPsURVLR-sTKtcTW8io3pQGwPLqqVeDIveIXkQQuVChfi4YIc" +
                     "\r\n\r\n" + buffer;

  client.println(requestBody);
  Serial.println(requestBody);

}









void postCashTransaction(int cash){
      char* receivedString = getOauthToken();
    String receivedToken = extractOauthToken(receivedString);
    postCashTransaction(receivedToken, cash);
}
