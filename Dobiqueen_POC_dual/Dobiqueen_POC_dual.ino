/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
//#include <SoftwareSerial.h>
#define SERIALDEBUG
// Replace with your network credentials
const char* ssid     = "You definitely want me";
const char* password = "tofunaan1629";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";
String output3State = "off";
String output2State = "off";
String output1State = "off";

// Assign output variables to GPIO pins
const int output5 = 2;
const int TX_LINE = 1;
//const int output4 = 4;


//SoftwareSerial mySerial(D0, D1); // RX, TX

void setup() {
  

  // Connect to Wi-Fi network with SSID and password
#ifdef SERIALDEBUG
  Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
 


    Serial.print(".");

  

  }
//  digitalWrite(output5, HIGH);
  
//   Print local IP address and start web server

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

#endif

  server.begin();
delay(500);
//Serial.end();
Serial.begin(9600);

//Serial.swap();
//mySerial.begin(9600);
//Serial1.begin(9600);
}

void loop(){
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
   // Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
   
    while (client.connected()) {            // loop while the client's connected
               if(Serial.available()>0)
                {
                  char incomingByte = Serial.read();
                  //sendCommand(incomingByte);
                   //mySerial.print(incomingByte);
                   Serial.print(incomingByte);
                  // Serial1.print(incomingByte);
    
                  }
      if (client.available()) {             // if there's bytes to read from the client,
 
        char c = client.read();             // read a byte, then
       // Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /1/on") >= 0) {
//              Serial.begin(9600);
//             Serial.println("@");
//             Serial.end();
//             pinMode(TX_LINE, OUTPUT);
//             digitalWrite(TX_LINE, LOW);
               sendCommand('@');
               // mySerial.print("@");
               //switchOff();
              output1State = "on";
             
            } else if (header.indexOf("GET /1/off") >= 0) {
              
              output1State = "off";
            
              
            } else if (header.indexOf("GET /2/on") >= 0) {
//              Serial.begin(9600);
//             Serial.println("B");
//             Serial.end();
//              pinMode(TX_LINE, OUTPUT);
//             digitalWrite(TX_LINE, LOW);
              sendCommand('B');
              //mySerial.print("B");
              // switchOff();
              output2State = "on";
             
            } else if (header.indexOf("GET /2/off") >= 0) {
            
              output2State = "off";
            
            }else if (header.indexOf("GET /3/on") >= 0) {
//              Serial.begin(9600);
//              Serial.println("C");
//               Serial.end();
//               pinMode(TX_LINE, OUTPUT);
//             digitalWrite(TX_LINE, LOW);
              sendCommand('C');
              //mySerial.print("C");
               //switchOff();
              output3State = "on";
              
            } else if (header.indexOf("GET /3/off") >= 0) {
              
              output3State = "off";
           
            }else if (header.indexOf("GET /4/on") >= 0) {
//              Serial.begin(9600);
//              Serial.println("F");
//               Serial.end();
//               pinMode(TX_LINE, OUTPUT);
//             digitalWrite(TX_LINE, LOW);

            sendCommand('F');
          //  mySerial.print("F");
               //switchOff();
              output4State = "on";
             
            } else if (header.indexOf("GET /4/off") >= 0) {
              
              output4State = "off";
             
            }else if (header.indexOf("GET /5/on") >= 0) {
//              Serial.begin(9600);
//              Serial.println("D");
//               Serial.end();
//               pinMode(TX_LINE, OUTPUT);
//             digitalWrite(TX_LINE, LOW);
               sendCommand('D');
              // mySerial.print("D");
              // switchOff();
              output5State = "on";
             
            } else if (header.indexOf("GET /5/off") >= 0) {
             
              output5State = "off";
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
      
            // Web Page Heading
            client.println("<body><h1>DobiQueen ePay Demo</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 5  
            client.println("<p> RM 1 </p>");
            // If the output5State is off, it displays the ON button       
            if (output1State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">RM 1</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">RM 1</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            client.println("<p> RM 5 </p>");
            // If the output4State is off, it displays the ON button       
            if (output2State=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">RM 5</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">RM 5</button></a></p>");
            }
            
               client.println("<p> RM 10 </p>");
            // If the output4State is off, it displays the ON button       
            if (output3State=="off") {
              client.println("<p><a href=\"/3/on\"><button class=\"button\">RM 10</button></a></p>");
            } else {
              client.println("<p><a href=\"/3/off\"><button class=\"button button2\">RM 10</button></a></p>");
            }

            client.println("<p> RM 20 </p>");
            // If the output4State is off, it displays the ON button       
            if (output4State=="off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">RM 20</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">RM 20</button></a></p>");
            }

            client.println("<p> RM 50 </p>");
            // If the output4State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">RM 50</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">RM 50</button></a></p>");
            }
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  //  Serial.println("Client disconnected.");
  //  Serial.println("");
  }
}
void sendCommand(char _input)
{
//  switchOn();
//  delay(50);
//  Serial.begin(9600);
  Serial.println(_input);
//  delay(50);
}

void switchOff(){
Serial.end();
pinMode(D10, OUTPUT );
digitalWrite(D10, LOW);
delay(50);

}
void switchOn(){
pinMode(D10, OUTPUT );
digitalWrite(D10, HIGH);  
}
