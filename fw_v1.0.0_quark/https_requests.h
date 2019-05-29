#define SERIALDEBUG 0 //HTTPS DEBUG. CHANGE VALUE TO 1 TO TURN IN ON
bool oauthFail = false;
/********************CAUTION: DO NOT CHANGE. *****************************************************************************/

//SERVER DETAILS
const char* host  PROGMEM = "dobiqueen.digitalforest.io";

//HTTPS REQUEST CREDENTIALS
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";

//OAUTH ACCESS CREDENTIALS
const int client_id_password_grant = 2;
const char* client_secret_password_grant  PROGMEM = "xj5CtbPW7LNXOE5AvwY7BUGgfjJ0HAH9fA2gBYMe";
/****************************************************************************************************************/
bool SuccessfulHttpRequest(WiFiClientSecure _client)
{
  String response  = _client.readStringUntil(':');
 
#if SERIALDEBUG
  Serial.print("Status code: ");
  Serial.print(response[9]);
  Serial.print(response[10]);
  Serial.println(response[11]);
#endif
//checking for status code 200 which is a successful response. Checking the first number for '2' is sufficient enough. Other error codes start with '4' e.g. 422
  if(response[9] == 2)
    {
      #if SERIALDEBUG
        Serial.print("Successful HTTPS Request");
      #endif
      return true;
    }
  else
  {
    #if SERIALDEBUG
      Serial.print("Successful HTTPS Request");
    #endif
    return false;
  }
}

char* getOauthToken(const char* _terminal, const char*  _tPassword)
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
    Serial.println("[getOauthToken]: connection failed");
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
  root["username"] = _terminal;
  root["user_type"] = "terminal";
  root["password"] = _tPassword;
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

}


String extractOauthToken(char* stringobject) {

  DynamicJsonBuffer  jsonBuffer(3000);
  JsonObject& root = jsonBuffer.parseObject(stringobject);

  if (!root.success())
  {
    oauthFail = true;
#if SERIALDEBUG
    Serial.println("[extractOauthToken]: parseObject() failed");
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


bool patchTransactionApproval(String access_token, int _WS_txID)
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

  String confirmation_url = transaction_approval_url + String(_WS_txID);
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
  bool statusCheck = SuccessfulHttpRequest(client);
   //String  line = client.readStringUntil(':'); //readString() is significantly slow
#if SERIALDEBUG
  Serial.println("PATCH REQ SENT");
  //Serial.println(line);
#endif

  if(statusCheck == true) return true;
  else return false;

}


bool postCashTransaction_internal(String access_token, int token, String _date, String _time) {

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
  root["transaction_at"] = _date + " " + _time;
  //ADD DATE TIME BASED ON THE FLAG
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

 bool statusCheck = SuccessfulHttpRequest(client);
   //String  line = client.readStringUntil(':'); //readString() is significantly slow
#if SERIALDEBUG
  Serial.println("POST REQ SENT");
#endif

  if(statusCheck == true) return true;
  else return false;
}


bool postCashTransaction(int cash, const char* _terminal, const char* _tPassword, String _date, String _time) {
  char* receivedString = getOauthToken(_terminal, _tPassword);
  String receivedToken = extractOauthToken(receivedString);
  //oauth parsing fails sometimes. This is a failsafe test // Only repeats it second time
  if(oauthFail)
  {
    receivedString = getOauthToken(_terminal, _tPassword);
    receivedToken = extractOauthToken(receivedString);
    oauthFail = false;
  }
  bool successfulReq = postCashTransaction_internal(receivedToken, cash, _date, _time);

  if(successfulReq == true) return true;
  else return false;
}

bool patchConfirmTransaction(const char* _terminal, const char* _tPassword, int _WS_txID) // maybe pass in the transaction ID and check inside for match
{
  char* receivedString = getOauthToken(_terminal, _tPassword);
  String receivedToken = extractOauthToken(receivedString);
  //oauth parsing fails sometimes. This is a failsafe test
  if(oauthFail)
  {
    receivedString = getOauthToken(_terminal, _tPassword);
    receivedToken = extractOauthToken(receivedString);
    oauthFail = false;
  }
  bool successfulReq = patchTransactionApproval(receivedToken, _WS_txID);
  
  if(successfulReq == true) return true;
  else return false;
  
}
