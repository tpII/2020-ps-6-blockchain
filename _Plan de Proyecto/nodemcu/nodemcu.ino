#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
//RFID
#include <SPI.h>
#include <MFRC522.h>

//RFID
#define SS_PIN 2  //D4
#define RST_PIN 0 //D3

//settings._______________________
//WIFI
String ssid     = "MovistarFibra-1806E0"; //wifi 2.4ghz config
String password = "ic3ksvsr5mDrYaJhRAPo"; 
int serverport = 80;
//SERVER
WiFiServer server(serverport);
String heroku_url = "https://peaceful-earth-53515.herokuapp.com";
String heroku_post_card = "post?";
HTTPClient http;
String header;
String clientCharTemp;
//RFID
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
//________________________________

//variable._____________________________
boolean wifiConnected = false;
int serverConnected = false;
String cardKey = "PLEASE SCAN A CARD";
//serial
char tempchar;
String menuCmd="";
String userInput="";
String jsonCardKey = "";
String httpCardKey = "";
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
//________________________________

//functions.________________________________
void webserver()
{
  if(serverConnected) {
 //server.handleClient();
 WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      currentTime = millis();
      previousTime = currentTime;
      while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
        currentTime = millis();         
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
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

              if (header.indexOf("GET /get?console=menu") >= 0) {
                  clientCharTemp="";
                  clientCharTemp+= "<br>_______________________________________"
                  "<br>COMMAND MENU"
                  "<br>type a command to execute. command list:"
                  "<br>menu : show menu"
                  "<br>wifiinfo : show connected wifi info"
                  "<br>scan : scan rfid card"
                  "<br>upload : send cardkey to server"
                  "<br>_______________________________________";
              } else if (header.indexOf("GET /get?console=wifiinfo") >= 0) {
                  clientCharTemp="";
                  clientCharTemp+= "<br>Connected to "+ ssid;
              } else if (header.indexOf("GET /get?console=scan") >= 0) {
                  scanRfidCard();
              } else if (header.indexOf("GET /get?console=upload") >= 0) {
                  writeCardkeyInHttp();
                  sendToWebsite();
              }
              
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\"></head>");
              
              // Web Page Heading
              client.println("<body><h1>ESP8266 Web Server</h1>");
              
              // content
              client.println("<p>Last scan: "+cardKey+"</p>");
              client.println("<form action=\"/get\"><input type=\"text\" name=\"console\"><br><br>");
              client.println("<input type=\"submit\" value=\"Submit\"></form>");  
              client.println("<br>___________________________________________________<br>");
              client.println("<p>"+clientCharTemp+"</p>");
              client.println("<br>___________________________________________________<br></body></html>");
              
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
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  }
}

// read the string from the serial monitor (the console)
void readUserInput()
{
  userInput="";
  while(!Serial.available()); //wait for input
  while(Serial.available()) {
    delay(15);
    tempchar=Serial.read();
    userInput+=tempchar;
  }
}
void readMenuCmd()
{
  while(Serial.available()) {
    delay(15);
    tempchar=Serial.read();
    menuCmd+=tempchar;
  }
}

void showMenuCmd()
{
  Serial.println("\n______________________________________________________");
  Serial.println("COMMAND MENU");
  Serial.println("type a command to execute. command list:");
  Serial.println("menu : show menu");
  Serial.println("wifi : connect to wifi");
  Serial.println("wifiinfo : show connected wifi info");
  Serial.println("server : start server");
  Serial.println("scan : scan a card");
  Serial.println("upload : upload card rfid to website");
  Serial.println("debug : show debug info");
  Serial.println("______________________________________________________");
}
void writeCardkeyInJson()
{
      //char* cardKeyJ = "";
      Serial.println("\nWriting JSON...");
      //StaticJsonDocument<JSON_OBJECT_SIZE(1)> doc; //json lib v6
      //doc["CardKey"] = cardKeyJ;
      //serializeJson(doc,jsonCardKey,112);
      jsonCardKey = "{\"Cardkey\":\"" + cardKey + "\"}";
      Serial.println("JSON DONE");
}
void writeCardkeyInHttp()
{
      Serial.println("\nWriting HTTP...");
      httpCardKey = "CARDKEY=" + cardKey;
      Serial.println("HTTP DONE");
}
void infoWifi()
{
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); //muestra ip asignada del nodemcu
}
// connect to wifi – returns true if successful or false if not
boolean connectWifi()
{
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA); //para conectarla a una red Wi-Fi a través de un AP
  Serial.println("\nWiFi Config, type !retry to reconnect or enter new SSID");
  Serial.println("TYPE SSID ");
  readUserInput();
  if(userInput != "!retry")
  {
    ssid=userInput;
    Serial.println("TYPE PASSWORD ");
    readUserInput();
    password=userInput;
  }
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PASSWORD: ");
    Serial.println(password);
    WiFi.begin(ssid, password); //se conecta
    Serial.println("");
    Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) { //si estado no conectado reintentar
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    infoWifi();
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

boolean connectWifiSetup()
{
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA); //para conectarla a una red Wi-Fi a través de un AP
    WiFi.begin(ssid, password); //se conecta
    Serial.println("");
    Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) { //si estado no conectado reintentar
    delay(500);
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  
  if (state){
    infoWifi();
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}

void sendToWebsite()
{
  String httpTempData;
  httpTempData="";
  Serial.println("DATA TO SEND: "); 
  Serial.println(httpCardKey); 
  Serial.print("Sending To Website: "); 
  Serial.println(heroku_url);
  httpTempData= httpCardKey;
  http.begin(heroku_url);
  http.addHeader("content-type","application/x-www-form-urlencoded");
  http.POST(httpTempData);
  //http.writeToStream(&Serial);
  http.end();
}

void scanRfidCard()
{
  Serial.println("\nPlace card in front of the RFID SCANNER");
  Serial.println("type exit to go back to menu");
  menuCmd="";
  //wait for card scan or select
  while ( (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial() ) && menuCmd != "exit") {
    readMenuCmd(); 
  }
  if(menuCmd != "exit")
  {
    //CHECK TYPE MIFARE type
    // Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    // Serial.println(rfid.PICC_GetTypeName(piccType));
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }
    //SCAN
    Serial.print("\nUID tag :");
    String cardKeyScanning= "";
    byte letter;
    for (byte i = 0; i < rfid.uid.size; i++) 
    {
       Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
       Serial.print(rfid.uid.uidByte[i], HEX);
       cardKeyScanning.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
       cardKeyScanning.concat(String(rfid.uid.uidByte[i], HEX));
    }
    cardKeyScanning.toUpperCase();
    Serial.println();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    cardKey = cardKeyScanning;
  }
 }

void showDebug()
{
  Serial.println("WIFI VAR:");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println("KEYCARD VAR:");
  Serial.println(cardKey);
  Serial.println(httpCardKey);
}


//________________________________

void setup() 
{
  Serial.begin(9600);
  
  //RFID
  SPI.begin();
  rfid.PCD_Init();
  wifiConnected = connectWifiSetup();
  if(wifiConnected)
  {
    Serial.println("Starting server...");
    server.begin();
    serverConnected=1;
  }
  delay(2000);//delay before kicking things off
  showMenuCmd();
}

void loop() 
{
  webserver();
  readMenuCmd();
  if(menuCmd=="menu") {
      showMenuCmd();
    }
  if(menuCmd=="wifi") {
      wifiConnected = connectWifi();
      showMenuCmd();
    }
  if(menuCmd=="wifiinfo") {
      infoWifi();
      showMenuCmd();
    }
  if(menuCmd=="server") {
      if(wifiConnected)
      {
        Serial.println("Starting server...");
        server.begin();
        serverConnected = 1;
        Serial.println("Server address: ");
        Serial.print(WiFi.localIP());
        Serial.print(":");
        Serial.println(serverport);
      } else {
        Serial.println("Wifi not connected!");
      }
      showMenuCmd();
    }
  if(menuCmd=="scan") {
      scanRfidCard();
      showMenuCmd();
    }
  if(menuCmd=="upload") {
      writeCardkeyInHttp();
      sendToWebsite();
      showMenuCmd();
    }
  if(menuCmd=="debug") {
      showDebug();
      showMenuCmd();
    }
  menuCmd="";
  delay(1000);
}
