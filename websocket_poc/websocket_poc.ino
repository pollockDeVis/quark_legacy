#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
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



/********************CAUTION: DO NOT CHANGE. *****************************************************************************/

//SERVER DETAILS
const char* host  PROGMEM = "dobiqueen.digitalforest.io";
//WEBSOCKET PARAMETERS
const int webSocketPort = 2052;
bool websocketReceivedEvent = false;
bool successfulTxn = false;
int rxTokens;
int WS_txID;
int WS_terminalID;


//HTTPS REQUEST CREDENTIALS
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";

//OAUTH ACCESS CREDENTIALS
const int client_id_password_grant = 2;
const char* client_secret_password_grant  PROGMEM = "xj5CtbPW7LNXOE5AvwY7BUGgfjJ0HAH9fA2gBYMe";

/***************************************************************************************************************/
#define USE_SERIAL Serial
#define SERIALDEBUG 1

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

void connectEvent(const char * payload, size_t length)
{
#if SERIALDEBUG
  Serial.println("EMITTING WEBSOCKETS");
  Serial.println("SENDING AUTHORIZATION WEBSOCKET");
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

void setup() {
  USE_SERIAL.begin(9600);



  USE_SERIAL.setDebugOutput(false);
#if SERIALDEBUG
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();


  Serial.print("Connecting to WiFi");
  Serial.println(ssid);
#endif

  for (uint8_t t = 4; t > 0; t--) {
#if SERIALDEBUG
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
#endif
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
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


  //Serial.println(receivedToken);
  webSocket.on("connect", connectEvent);
  webSocket.on("transaction", event);
  webSocket.begin(host, webSocketPort);
}

void loop() {


  if (Serial.available() > 0)
  {
    char incomingByte = Serial.read();
    Serial.println(incomingByte);
    int cashValue = detokenizer(incomingByte);
    if (cashValue > 0) {
      postCashTransaction(cashValue);
    }
  }


  webSocket.loop();

  if (websocketReceivedEvent == true)
  {
    websocketReceivedEvent = false;

#if SERIALDEBUG
    Serial.print("TOKENS  :");
    Serial.println(rxTokens);
#endif

    //CHECK TERMINAL ID #TODO
    tokenizer(rxTokens);

  if(successfulTxn)
  {
  #if SERIALDEBUG   
    Serial.print("Boolean is True ");
    Serial.println(" Making the flag false");
  #endif
   patchConfirmTransaction();
   successfulTxn = false;

  }
}
}
char* getOauthToken()
{
  String token_url   = "/api/v1/oauth/token";

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


#if SERIALDEBUG
  Serial.print("requesting URL: ");
  Serial.println(token_url);
#endif
  DynamicJsonBuffer jsonBuffer;


  JsonObject& root = jsonBuffer.createObject();
  root["client_id"] = client_id_password_grant;
  root["client_secret"] = client_secret_password_grant;
  root["grant_type"] = "password";
  root["scope"] = "*";
  root["username"] = TERMINAL;
  root["user_type"] = "terminal";
  root["password"] = TERMINAL_PASSWORD;
  int length = root.measureLength();
  char buffer[length + 1];
  root.printTo(buffer, sizeof(buffer));

  String requestBody = "POST " + token_url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + root.measureLength() +
                       "\r\n\r\n" + buffer;

  client.println(requestBody);
#if SERIALDEBUG
 // Serial.println(requestBody);
  Serial.println("request sent");
#endif
  String  line = client.readStringUntil('}'); //readString() is significantly slow
  //  char* pch;
  //  pch = strchr(line, '{');
  line = line + '}';
#if SERIALDEBUG
 // Serial.println(line);
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

  arraysize = tailcount - headcount + 1;
  char tempBufferforJSON[arraysize];
  for (int counter = headcount; counter < tailcount + 1; counter++) {
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
String extractOauthToken(char* stringobject) {

  DynamicJsonBuffer  jsonBuffer(3000);
  JsonObject& root = jsonBuffer.parseObject(stringobject);

  if (!root.success())
  {
#if SERIALDEBUG
    Serial.println("parseObject() failed");
#endif
    //return;
  }
  String access_token = root["access_token"];
#if SERIALDEBUG
  Serial.print("Access Token :");
  //Serial.println(access_token);
#endif
  return access_token;
}



void patchTransactionApproval(String access_token)
{
  WiFiClientSecure client;
  String transaction_approval_url  = "/api/v1/transactions/" ; 
  client.setFingerprint(fingerprint);
#if SERIALDEBUG
  Serial.println("PATCH REQUEST");
  Serial.print("connecting to ");
  Serial.println(host);
  Serial.printf("Using fingerprint '%s'\n", fingerprint);
#endif

  if (!client.connect(host, httpsPort)) {
#if SERIALDEBUG
    Serial.println("connection failed");
#endif
    //return "X";
  }

  String confirmation_url = transaction_approval_url + String(WS_txID);
#if SERIALDEBUG
  Serial.print("requesting URL: ");
  Serial.println(confirmation_url);
#endif
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["action"] = "complete";

  int length = root.measureLength();
  char buffer[length + 1];
  root.printTo(buffer, sizeof(buffer));

  String requestBody = "PATCH " + confirmation_url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "Content-Length: " + root.measureLength() +
                       "\r\nAuthorization: " + "Bearer " + access_token +
                       "\r\n\r\n" + buffer;

  client.println(requestBody);
#if SERIALDEBUG
  Serial.println("PATCH REQ SENT");
#endif

}


void postCashTransaction(String access_token, int token) {

  WiFiClientSecure client;
  String cash_url   = "/api/v1/cash-transactions";
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
  // root["terminal"] = client_id;
  root["tokens"] = token;
  int length = root.measureLength();
  char buffer[length + 1];
  root.printTo(buffer, sizeof(buffer));



  String requestBody2 = "POST " + cash_url + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" +
                        "Content-Type: application/json\r\n" +
                        "Content-Length: " + root.measureLength() +
                        "\r\nAuthorization: " + "Bearer " + access_token +
                        "\r\n\r\n" + buffer;

  client.println(requestBody2);
#if SERIALDEBUG
  //Serial.println(requestBody2);
#endif
  String  line2 = client.readStringUntil('}'); //readString() is significantly slow
#if SERIALDEBUG
  Serial.println(line2);
#endif
}


void postCashTransaction(int cash) {
  char* receivedString = getOauthToken();
  String receivedToken = extractOauthToken(receivedString);
  postCashTransaction(receivedToken, cash);
}

void patchConfirmTransaction() // maybe pass in the transaction ID and check inside for match
{
  char* receivedString = getOauthToken();
  String receivedToken = extractOauthToken(receivedString);
  patchTransactionApproval(receivedToken);
  
}
