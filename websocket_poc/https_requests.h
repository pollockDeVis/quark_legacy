//SERVER DETAILS
const char* host  PROGMEM = "dobiqueen.digitalforest.io";


//HTTPS REQUEST CREDENTIALS
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "a8 0e 9c 81 2a a8 e3 0f e8 3b f5 e6 4c 73 7c 07 a4 e7 cc e5";

//OAUTH ACCESS CREDENTIALS
const int client_id_password_grant = 2;
const char* client_secret_password_grant  PROGMEM = "xj5CtbPW7LNXOE5AvwY7BUGgfjJ0HAH9fA2gBYMe";


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



void patchTransactionApproval(String access_token, int _WS_txID)
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


void postCashTransaction(int cash, const char* _terminal, const char* _tPassword) {
  char* receivedString = getOauthToken(_terminal, _tPassword);
  String receivedToken = extractOauthToken(receivedString);
  postCashTransaction(receivedToken, cash);
}

void patchConfirmTransaction(const char* _terminal, const char* _tPassword, int _WS_txID) // maybe pass in the transaction ID and check inside for match
{
  char* receivedString = getOauthToken(_terminal, _tPassword);
  String receivedToken = extractOauthToken(receivedString);
  patchTransactionApproval(receivedToken, _WS_txID);
  
}
