#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "tokenizer.h"
#include "https_requests.h"
#include <SocketIoClient.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <ESP8266Ping.h>
/********************CHANGE THE PARAMS BELOW BEFORE INSTALLATION *****************************************************************************/
//QUARK PARAMETERS
const char* TERMINAL    PROGMEM = "DB000001";
const int   TERMINAL_ID PROGMEM = 1;
const char* TERMINAL_PASSWORD  PROGMEM = "123456";
const char* FIRMWARE_VERSION = "1.0.1"; 
const char* HARDWARE_VERSION = "1.0.0";

//WIFI CREDENTIALS
const char* ssid PROGMEM = "dobiqueen";
const char* password PROGMEM = "dobiqueen";
const unsigned long wifi_timeout PROGMEM = 10000; // 10 seconds waiting for connecting to wifi
const unsigned long wifi_reconnect_time PROGMEM = 120000; // 2 min retrying
unsigned long wifi_last_connected_time = millis();

bool WIFI_ACTIVE = true;
bool INTERNET_ACTIVE = true;
bool WIFI_reconnect_flag = false;
int accumulated_txns_wifi_reconnect = 0;
bool ACCUMULATED_TXNS = false;
//DEBUG
#define SERIALDEBUG 1 //WEBSOCKETS DEBUG. CHANGE VALUE TO 1 TO TURN IN ON
 
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
/***********************OFFLINE TXNS************************************************************************/
String DATE_TIME_ARRAY[20];
int OFFLINE_CASH_VALUE[20];
uint8_t offlineCounter = 0;
bool OfflineCashTransaction = false;
/***********************NTP SERVER************************************************************************/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const int secondInterval PROGMEM= 1000;
unsigned long updateInterval PROGMEM = 3600000; //update every hour
unsigned long last_millis = 0;
unsigned long offlineUNIXClock = 0;
unsigned long lastUpdateTime = 0;
/***************************************************************************************************************/
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;
Ticker ticker; //Ticker for Interrupt
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
/*******************************NTP SERVER SETUP**************************************************************/
   timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800 //Malaysian Time GMT +8
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(28800);

  offlineCounter = 0;

offlineUNIXClock = getUNIXTimeStamp();
lastUpdateTime  = 0;
ticker.attach(1, checkSerialISR); //(time in seconds, isrFunction)
} //end setup

void loop() 
{
  
 WIFI_ACTIVE = wifiStatusCheck();
 INTERNET_ACTIVE = internetConnectivity();

  if(millis()-last_millis > secondInterval) //Update timer
  {
     offlineUNIXClock += ((millis()-last_millis)/1000); //offlineUNIXClock++;-
    last_millis = millis();
   
    #if SERIALDEBUG
      Serial.println(offlineUNIXClock);
    #endif
  }
/***********************CHECK CASH TXNS**************************************************************/
  if (incomingFlag)
  {
    checkCashTransaction();

  }
    
  if(WIFI_ACTIVE == true)
  {
        ///////////////////////////  TIME SYNC ////////////////////////////////////////////////////////
      if(millis()-lastUpdateTime > updateInterval)
      {
            #if SERIALDEBUG
              Serial.println("Regular Time Sync");
            #endif
        lastUpdateTime = millis();
        offlineUNIXClock = getUNIXTimeStamp();
      }
    /************CHECK OFFLINE PENDING TXNS******************************************************************/
      if(OfflineCashTransaction == true)
      {
        if(INTERNET_ACTIVE == true)
        {
          postOfflineCashTransaction();
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
        successfulPATCH = patchConfirmTransaction(TERMINAL, TERMINAL_PASSWORD, WS_txID);
        successfulTxn = false;
        if(successfulPATCH == false)
        {
          //Record WS_TXID for updating later //#TODO To be Implemented Later
        }
      }
    }
  }
  else
  {
    if((millis() - wifi_last_connected_time) > wifi_reconnect_time && incomingFlag == false) // reconnects every 2 minutes || Serial is checked so that it doesn't go into reconnecting the wifi when transaction is taking place
    {
      WIFI_reconnect_flag = true;
      wifi_last_connected_time = millis(); // Readjusting the timer so that it doesn't keep on reconnecting on every loop after 2 min has elapsed
    #if SERIALDEBUG
      Serial.print("Connection Lost, Reconnecting");
    #endif
      connectWifi();
        WIFI_reconnect_flag = false;
    }
  }
}// end Loop



void checkSerialISR()
{
  if (Serial.available() > 0)
  {
//     #if SERIALDEBUG
//      Serial.print("Serial Availability: ");
//      Serial.println(Serial.available());
//    #endif
    incomingByte = Serial.read();
    Serial.println(incomingByte);
   
    incomingFlag = true;
    if(WIFI_reconnect_flag)
    {
      accumulated_txns_wifi_reconnect += detokenizer(incomingByte);
      ACCUMULATED_TXNS = true;
      //checkCashTransaction();
    }
  }
}
void checkCashTransaction()
{

      incomingFlag = false;

    int cashValue = detokenizer(incomingByte);

    if(ACCUMULATED_TXNS)     //Check for possible bugs
    {
      cashValue += accumulated_txns_wifi_reconnect;
      ACCUMULATED_TXNS = false;
      accumulated_txns_wifi_reconnect = 0;
      
    }
    if (cashValue > 0) 
    {
        if(WIFI_ACTIVE == true && INTERNET_ACTIVE == true)
        {
          //create a variable offline for TXN
          String _NTPString = updateTimeFromNTP();
          String _date = extractDate(_NTPString);
          String _time = extractTime(_NTPString);
          successfulPOST =  postCashTransaction(cashValue, TERMINAL, TERMINAL_PASSWORD, _date, _time);
        }//else store in offline array // create a function. This will be called for every transaction that is missed irrespective of the cause
        else
        {
          #if SERIALDEBUG
            Serial.println("[WIFI UNAVAILABLE] Recording Offline Transaction");
          #endif
          recordOfflineCashTransaction(cashValue);
        }
      //}
      if(successfulPOST == false)
      {
        #if SERIALDEBUG
          Serial.println("[UNSUCCESSFUL POST] Recording Offline Transaction");
        #endif
        //Take time stamp and record Cash value to permanent memory for later updates
        recordOfflineCashTransaction(cashValue);
      }
    }
}
void recordOfflineCashTransaction(int _cashValue) //HIGH LEVEL
{
      OfflineCashTransaction = true;
      #if SERIALDEBUG  
        Serial.println("[OFFLINE CASH TXN] ");
      #endif
      DATE_TIME_ARRAY[offlineCounter] = getDateTimeFromUnix(offlineUNIXClock); //updateTimeFromNTP();

      #if SERIALDEBUG  
        Serial.print("[NTP] ");
        Serial.println(DATE_TIME_ARRAY[offlineCounter]);
      #endif
      OFFLINE_CASH_VALUE[offlineCounter] = _cashValue;
      offlineCounter++;
      
      if(offlineCounter > 19) //sums up all and put it in the first array if it overflows
      {
        int cashSum = 0;
        for(uint8_t counter = 0; counter <= 19; counter++) // Adding Up all the values
        {
          cashSum += OFFLINE_CASH_VALUE[counter]; 
        }
        offlineCounter = 0;
        memset(DATE_TIME_ARRAY, 0, sizeof(DATE_TIME_ARRAY));
        memset(OFFLINE_CASH_VALUE, 0, sizeof(OFFLINE_CASH_VALUE));
        DATE_TIME_ARRAY[offlineCounter] = getDateTimeFromUnix(offlineUNIXClock);//updateTimeFromNTP();
        OFFLINE_CASH_VALUE[offlineCounter] = cashSum;
        offlineCounter++;
      }
}

void postOfflineCashTransaction() //HIGH LEVEL
{

  if(offlineCounter > 0)
  {
    offlineCounter--;
    int _cashValue = OFFLINE_CASH_VALUE[offlineCounter];
    String NTPResponse = DATE_TIME_ARRAY[offlineCounter];
    String _date = extractDate(NTPResponse);
    String _time = extractTime(NTPResponse);
    successfulPOST =  postCashTransaction(_cashValue, TERMINAL, TERMINAL_PASSWORD, _date, _time);
    if(offlineCounter == 0)
    {
      OfflineCashTransaction = false;
    }
  }

  
}

unsigned long getUNIXTimeStamp()
{
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  unsigned long epochTime =  timeClient.getEpochTime();
  return epochTime;
}

String getDateTimeFromUnix(unsigned long _unixtimestamp)
{
  String dateTime = timeClient.getFormattedDate(_unixtimestamp);
  return dateTime;
}
String updateTimeFromNTP()
{
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  String formattedDate = timeClient.getFormattedDate();
  return formattedDate;
   #if SERIALDEBUG
  Serial.println(formattedDate);
  Serial.print("Size of the string date: ");
  Serial.println(sizeof(formattedDate));
  #endif
}

String extractDate(String _NTPResponse)
{
  int splitT = _NTPResponse.indexOf("T");
  String dayStamp = _NTPResponse.substring(0, splitT);
  return dayStamp;
}

String extractTime(String _NTPResponse)
{
  int splitT = _NTPResponse.indexOf("T");
  String timeStamp = _NTPResponse.substring(splitT+1, _NTPResponse.length()-1);
  return timeStamp;
}

bool internetConnectivity()
{
      bool ret = Ping.ping(host, 1);
      #if SERIALDEBUG
      Serial.print("Ping Response : ");
      Serial.println(ret);
      #endif
      if(ret == false)
      {
        return false;
      }else
      {
        return true; 
      }
}
bool wifiStatusCheck()
{
  if(WiFiMulti.run() == WL_CONNECTED)
  {         
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
    Serial.print("Connecting to WiFi: ");
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
