/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/



#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
/* Test */
#include "RestClient.h"
/* .... */
#include <ArduinoJson.h>
#ifndef STASSID
#define STASSID "You know you want me"
#define STAPSK  "tofunaan1629"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "dobiqueen.digitalforest.io";
const int httpsPort = 443;

//RestClient client2 = RestClient("dobiqueen.digitalforest.io", 443, "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5");

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";
//const char* payload = "{\"client_id\":1,\"client_secret\":\"0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ\",\"grant_type\":\"client_credentials\",\"scope\":\"read\"}"; 
//String testReqst = 

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/api/v1/oauth/token";
  Serial.print("requesting URL: ");
  Serial.println(url);
  
  DynamicJsonBuffer jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  root["client_id"] = 1;
  root["client_secret"] = "0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ";
  root["grant_type"] = "client_credentials";
  root["scope"] = "read";
int length = root.measureLength();
  char buffer[length+1];
  root.printTo(buffer, sizeof(buffer));
//Serial.println(buffer);
//Content-Type: application/json
//client.addHeader("Content-Type", "application/json");
String requestBody = "POST " + url + " HTTP/1.1\r\n" + 
                     "Host: " + host + "\r\n" + 
                     "Content-Type: application/json\r\n" + 
                     "Content-Length: " + root.measureLength() +
                     "\r\n\r\n" + buffer; //
//                     "client_id=1&client_secret=0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ&grant_type=client_credentials&scope=read";

client.println(requestBody);

Serial.println(requestBody);
//HTTPClient http; //Declare object of class HTTPClient
//http.begin("https://dobiqueen.digitalforest.io/api/v1/oauth/token", "fingerprint"); //Specify request destination
//http.addHeader("Content-Type", "application/json"); //Specify content-type header
//http.POST(buffer);
//http.writeToStream(&Serial);
//String response = http.getString();
//Serial.println(response);
//http.end();
//     client.print("POST /api/v1/oauth/token? HTTP/1.1\nHost: dobiqueen.digitalforest.io\nContent-Type: application/json\ncache-control: no-cache\n{\"client_id\":1,\"client_secret\":\"0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ\",\"grant_type\":\"client_credentials\",\"scope\":\"read\"}------WebKitFormBoundary7MA4YWxkTrZu0gW--");
//     Serial.println(requestBody);        
//    "Content-Type: " +  "application/json\r\n\r\n" +
  Serial.println("request sent");
  
//    DynamicJsonBuffer jsonBuffer(1000);
//
//  // Parse JSON object
//  JsonObject& root = jsonBuffer.parseObject(client);
//  
//  if (!root.success()) {
//    Serial.println(F("Parsing failed!"));
//    return;
//  }
//  while (client.connected()) {
//    String line = client.readString();
//    if (line == "{") {
//      
//      break;
//    }
//  }
String  line = client.readStringUntil('}'); //readString() is significantly slow
       Serial.println(line);

  //String test = client.getString();
String url2 = "/api/v1/cash-transactions";

//DynamicJsonBuffer jsonBuffer2;
//  
//  JsonObject& root2 = jsonBuffer2.createObject();
//  root2["terminal"] = 1;
//  root2["tokens"] = "77";
//  
//int length2 = root2.measureLength();
//  char buffer2[length2+1];
//  root.printTo(buffer2, sizeof(buffer2));


String requestBody2 = "POST " + url2 + " HTTP/1.1\r\n" + 
                     "Host: " + host + "\r\n" + 
                     "Content-Type: application/json\r\n" + 
                     "Content-Length: 26"  +
                     "\r\nAuthorization: eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsImp0aSI6IjlkMTA3Njc4OTRhOTE3N2Y4YjMxNTg1NDEwY2NlN2Q5NDY1MTBiMmUwMjBjODU4ODRjYjgzOWVmYzY3MzI1NDA2NGQzNTYyMzU1NjhmYTQ2In0.eyJhdWQiOiIxIiwianRpIjoiOWQxMDc2Nzg5NGE5MTc3ZjhiMzE1ODU0MTBjY2U3ZDk0NjUxMGIyZTAyMGM4NTg4NGNiODM5ZWZjNjczMjU0MDY0ZDM1NjIzNTU2OGZhNDYiLCJpYXQiOjE1NTA3ODI3MDAsIm5iZiI6MTU1MDc4MjcwMCwiZXhwIjoxNTgyMzE4NzAwLCJzdWIiOiIiLCJzY29wZXMiOlsicmVhZCJdfQ.aIxlTMueL0O_MwqLom4YD8hEnfBpEwEL-OcVKzVcpXZOT2hdJLKPRd1H4sgF2ToYCEHnVTLVHDaQS7-b7gtQY6tEjYUhucP_Ipbc5l0y11G2yij6GriR9Opnp4MrTuP7j9L1fZdBFeT1f0XpsXoKaf95Xubp-h4fUfJf8pzCE4i-eNYel-JEK-8RHu05Lc9bm0tEPVlQD6LqQnyU9zlywOpdCXv_tKzq5qvZS8b4hMxLJXc88iobbiUAOfYX2Rj1k6NH8gm6rbTd7wTkFHh23q16P4avR8yHzJVQtEisZQ3kFBOb3IWOQd_9t6r_E9uVOfU-_8B9lk6uhHAY2b47A0l6Y8ebAJqQLmqmcNA7Rt8BPNFHkfk-CxvphH1Pq3iCm52qIenZTx8efELINQuNiqLQicyz0wfzEfJYoLP_h_VHshSacC6CRCybL0a9PzZAtI2LO_GYJZ-Kbm8_9u5eeB5W_np57B5qmVLv3-kCi37TApVAkat07PpCyjpNfOXzb34fMaDNpC7Qac5c-G0isVp5_rexI1pK-yOBQCNmQsZmJPKA8lTSI6_nyJ_Ms2PIyS_ZfiVWR1pnblNUUJO9OhXpDpSjPBLGRCVvfa0Nusds3i3EnaVQhpTuTz0c6Sk5RHNgayWo7lmO6OUR4aiDFvxy1M1Rv0ORhbaeCZy3Voo"
                     + "\r\n\r\n{\"terminal\":1,\"tokens\":5}"; //
//                     "client_id=1&client_secret=0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ&grant_type=client_credentials&scope=read";

client.println(requestBody2);
Serial.println(requestBody2);
 String  line2 = client.readStringUntil('}'); //readString() is significantly slow
       Serial.println(line2);
  /*
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
  */
//String response2 = "";
//int statusCode = client2.post("/api/v1/oauth/token", buffer, &response2);
//  Serial.println(statusCode);
//  Serial.print("Response body from server: ");
//  Serial.println(response);

}

void loop() {
}

/*
POST /api/v1/oauth/token? HTTP/1.1
Host: dobiqueen.digitalforest.io
Content-Type: application/json
cache-control: no-cache
{"client_id":1,"client_secret":"0KCsioBv1YQHVco8vtemlq2UfGEQj4k1m6byLaIQ","grant_type":"client_credentials","scope":"read"}------WebKitFormBoundary7MA4YWxkTrZu0gW--

*/
