//INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ESP8266WiFi.h> //    .INCLUDES.
#include <ESP8266WebServer.h>  //nodemcu webserver
#include <ESP8266HTTPClient.h> //http comm
#include <ArduinoJson.h> //json
#include <SPI.h> //RFID
#include <MFRC522.h> //RFID
#include <time.h> //datetime

//DEFINE
#define SS_PIN 2  //RFID D4  .DEFINITIONS.
#define RST_PIN 0 //D3

//SETTINGS
//WIFI SETTINGS
String ssid     = "MovistarFibra-1806E0"; //wifi 2.4ghz config   .SETTINGS.
String password = "ic3ksvsr5mDrYaJhRAPo"; //default wifi AP

//NODE SETTINGS
String nodeurl = "http://ps6blockchain.herokuapp.com/"; //http only, should be best node for less lag
String recipientAddress = "8422ce0f1fcb49b18d3687c4821346de"; //default recipient uuid, can be any node

//NODEMCU SETTINGS
const String location="Neuquen";

//WEBSERVER
int serverport = 80; //WEB SERVER PORT

//VAR
//id and ip
String chipid;
String chipip; //use wifiinfo to find your webserver ip
//WEBSERVER
WiFiServer server(serverport); //nodemcu webserver
String clientHeader;
String clientCharTemp;

//RFID SCANNER
MFRC522 rfid(SS_PIN, RST_PIN); //RFID set pins
MFRC522::MIFARE_Key key; //key 
//other
boolean wifiConnected = false;  // .VARIABLES.
int serverConnected = false; //nodemcu server on?
String cardKey = "PLEASE SCAN A CARD"; //cardkey stored here
char tempchar; //serial temp
String menuCmd=""; //user pick menucmd
String userInput=""; //user console input
String jsonArgs = ""; //json data to send
//String httpArgs = "";

//TIME VAR
unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0;
const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

//FUNCTIONS
void nodemcuWebserver(); //starts the nodemcu webserver                   //.FUNCTIONSD.
void consoleMenu(); //console menu
void readUserInput();  //read user input from console
void readUserMenuCmd();    //read user command from console
String showMenu();    //show the menu in console
void writeTx();  //write tx data in JSON
void setTxRecipient(); //set recipient of tx (if not set recipient is default)
void sendTx();    //send tx to node
String wifiInfo();  //show wifi information
boolean connectWifi();  //connect to any wifi AP
boolean connectWifiSetup(); //connect to default wifi AP
void scanRfidCard(); //scan a rfid card
void showDebug();  //show var info
String startServer(); //starts the nodemcu webserver
char** str_split(char* a_str, const char a_delim); //string splitter to parse http get args
//void writeTxHttp(); //http version

//PROGRAM
void setup() //.PROGRAM.
{
	//SERIAL CONSOLE
	Serial.begin(9600);
	Serial.setDebugOutput(true);

	//CHIP ID
	char chipidt[32];
	itoa(ESP.getChipId(),chipidt,10);
	chipid = chipidt;

	//RFID INIT
	SPI.begin();
	rfid.PCD_Init();

	//WIFI
	while(wifiConnected == false)
	{
		wifiConnected = connectWifiSetup(); //connect to default wifi
	}

	//NODEMCU SERVER
	startServer();

	//SHOW MENU AT START
	delay(2000);//delay before kicking things off
	showMenu();
}

void loop()
{
	nodemcuWebserver(); //user can send cmd using the webserver
	//consoleMenu()
	delay(1000);
}

//FUNCTIONS
/**
 * console menu if you are using the console instead of the webserver
 */
void consoleMenu()
{
	readUserMenuCmd(); //user can send cmd using the arduino console
	 //show menu
	if(menuCmd=="menu") {
		showMenu();
	}
	//connect to any wifi AP
	if(menuCmd=="wifi") { 
		wifiConnected = connectWifi();
		showMenu();
	}
	//show wifi ssid and ip
	if(menuCmd=="wifiinfo") { 
		wifiInfo();
		showMenu();
	}
	//restart nodemcu webserver
	if(menuCmd=="server") {
		startServer();
		showMenu();
	}
	//scan a rfid card
	if(menuCmd=="scan") {
		scanRfidCard();
		showMenu();
	}
	//set recipient for txs
	if(menuCmd=="sendto") {
		setTxRecipient();
		showMenu();
	}
	//send tx
	if(menuCmd=="upload") {
		writeTx();
		sendTx();
		showMenu();
	}
	//show variables
	if(menuCmd=="debug") {
		showDebug();
		showMenu();
	}
	menuCmd="";
}
/**
 * starts the nodemcu webserver. use the nodemcu ip address + port to access the site in any browser
 */
void nodemcuWebserver()  //.FUNCTIONS.
{
	if(serverConnected) {
		//server.handleClient();
		WiFiClient client = server.available();   // Listen for incoming clients

		if (client) {                             // If a new client connects,
			Serial.println("New Client.");          // print a message out in the serial port
			String currentLine = "";                // make a String to hold incoming data from the client
      String currentform = "console";
			currentTime = millis();
			previousTime = currentTime;
			while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
				currentTime = millis();
				if (client.available()) {             // if there's bytes to read from the client,
					char c = client.read();             // read a byte, then
					Serial.write(c);                    // print it out the serial monitor
					clientHeader += c;
					if (c == '\n') {                    // if the byte is a newline character
						// if the current line is blank, you got two newline characters in a row.
						// that's the end of the client HTTP request, so send a response:
						if (currentLine.length() == 0) {
							// HTTP clientHeaders always start with a response code (e.g. HTTP/1.1 200 OK)
							// and a content-type so the client knows what's coming, then a blank line:
							client.println("HTTP/1.1 200 OK");
							client.println("Content-type:text/html");
							client.println("Connection: close");
							client.println();

              //clean our client console if too full
              if(strlen(clientCharTemp.c_str()) > 512)
              {
                clientCharTemp="";
              }

							//print data in our homemade console
							if (clientHeader.indexOf("GET /get?console=menu") >= 0) {
								clientCharTemp+= showMenu();
							} else if (clientHeader.indexOf("GET /get?console=wifiinfo") >= 0) {
								clientCharTemp+= wifiInfo();
							} else if (clientHeader.indexOf("GET /get?console=server") >= 0) {
								clientCharTemp+= startServer();
							} else if (clientHeader.indexOf("GET /get?console=scan") >= 0) {
								scanRfidCard();
								clientCharTemp+= "<br> Scan command completed";
							} else if (clientHeader.indexOf("GET /get?console=sendto") >= 0) {
                clientCharTemp+= "<br>!THIS COMMAND IS NOT WORKING YET!";
								clientCharTemp+= "<br>Type recipient of tx and website to carry it";
								clientCharTemp+= "<br>[recipient]_[url]_";
                clientCharTemp+= "<br>Dont forget the _ between arguments and at the end, dont write the brackets";
								clientCharTemp+= "<br>EXAMPLE: 8422ce0f1fcb49b18d3687c4821346de_http://ps6blockchain.herokuapp.com/_";
                //currentform = "sendto";
							} else if (clientHeader.indexOf("GET /get?sendto=") >= 0) {
								
								//parse http arguments
                const char* clientHeaderstr = clientHeader.c_str();
								char *b = strstr(clientHeaderstr, "sendto="); //find pos of args, sendto= is 7 char
								int pos = b - clientHeaderstr + 7;
								
								String args = clientHeaderstr + pos;
                char* argsstr;
                strcpy(argsstr,args.c_str());
								char** tokens;
								tokens = str_split(argsstr, '_');
								
								recipientAddress=*(tokens);
								nodeurl=*(tokens + 1);
								free(*(tokens));
								free(*(tokens+1));
								free(*(tokens+2));
								free(tokens);

								clientCharTemp+= "<br>CHANGING WEBSITE TO: ";
								clientCharTemp+= nodeurl;
								clientCharTemp+= "<br>CHANGING RECIPIENT TO: ";
								clientCharTemp+= recipientAddress;
							} else if (clientHeader.indexOf("GET /get?console=upload") >= 0) {
								writeTx();
								sendTx();
								clientCharTemp+= "<br> Transaction command completed";
                
								clientCharTemp+= "<br> Data sent to " + nodeurl;
								clientCharTemp+= "<br>";
								clientCharTemp+= jsonArgs;
							}

							// Display the HTML web page 
							// this part can be cleaned up with using SPIFFS and html files, check out:
							//https://tttapa.github.io/ESP8266/Chap11%20-%20SPIFFS.html
							client.println("<!DOCTYPE html><html>");
							client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
							client.println("<link rel=\"icon\" href=\"data:,\"></head>");

							// Web Page Heading
							client.println("<body><h1>ESP8266 Web Server</h1>");

							// content
							client.println("<p>Last scan: "+cardKey+"</p>");
              client.println("<p>command: "+currentform+"</p>");
							client.println("<form action=\"/get\"><input type=\"text\" name=\""+currentform+"\"><br><br>");
							client.println("<input type=\"submit\" value=\"Submit\"></form>");
							client.println("<br>______________________________________________________________________________________________________<br>");
							client.println("<p>"+clientCharTemp+"</p>");
							client.println("<br>______________________________________________________________________________________________________<br></body></html>");

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
			// Clear the clientHeader variable
			clientHeader = "";
			// Close the connection
			client.stop();
			Serial.println("Client disconnected.");
			Serial.println("");
		}
	}
}

/**
 * read the string from the serial monitor (the console)
 */
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
/**
 * read a cmd from the console
 */
void readUserMenuCmd()
{
	while(Serial.available()) {
		delay(15);
		tempchar=Serial.read();
		menuCmd+=tempchar;
	}
}
/**
 * show the menu, returns menu as string and prints all possible menucmd in the console
 */
String showMenu()
{
	String menustr;
	menustr =  "\n<br>______________________________________________________";
	menustr += "\n<br>COMMAND MENU";
	menustr += "\n<br>type a command to execute. command list:";
	menustr += "\n<br>menu : show menu";
	menustr += "\n<br>wifi : connect to wifi(not on webserver)";
	menustr += "\n<br>wifiinfo : show connected wifi info";
	menustr += "\n<br>server : start or restart server";
	menustr += "\n<br>scan : scan a card, the LED of the nodemcu will turn blue and wait for a scan";
	menustr += "\n<br>sendto : set the website and recipient to send data";
	menustr += "\n<br>upload : upload card rfid to website";
	menustr += "\n<br>debug : show debug info (not on webserver)";
	menustr += "\n<br>______________________________________________________";
	Serial.println(menustr);
	return menustr;
}
/**
 * start the nodemcu webserver
 */
String startServer()
{
	String startserverstr;
	startserverstr="";
	if(wifiConnected)
	{
		startserverstr+="\n<br>Starting server...";
		server.begin();
		serverConnected = 1;
		startserverstr+= "\n<br>Server address: ";
		startserverstr+= chipip;
		startserverstr+= ":";
		startserverstr+= serverport;
	} else {
		startserverstr+="\n<br>Wifi not connected!";
	}
	Serial.println(startserverstr);
	return startserverstr;
}
/**
 * set the recipient for the next txs and the website to use to send the tx
 */
void setTxRecipient()
{
	Serial.println("TYPE NODE URL "); //node receive the data and send to address of recipient node
	readUserInput();
	nodeurl=userInput;
	Serial.println("TYPE RECIPIENT ADDRESS ");
	readUserInput();
	recipientAddress=userInput;
}
/**
 * prints current time
 */
String datetimenow()
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char now[32];
	sprintf(now,"%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return now;
}
/**
 * prepare the transaction(tx) data in json format. parameters:
 * sender, recipient, amount, cardkey, location, date
 */
void writeTx()
{
  jsonArgs="";
	Serial.println("\nWriting JSON...");
	StaticJsonDocument<512> doc; //json lib v6
	doc["sender"] = chipid; //chip address
	doc["recipient"] = recipientAddress; //send to node address
	doc["amount"] = 1; //all amounts can be added to show how many times card was scanned
	doc["cardkey"] = cardKey;
	doc["location"] = location;
	doc["date"] = datetimenow(); //unix time
	serializeJson(doc,jsonArgs);
	Serial.println(jsonArgs);
	Serial.println("JSON DONE");
	//jsonCardKey = "{\"Cardkey\":\"" + cardKey + "\"}";
}
/**
 * prepare the transaction(tx) data in http get format. parameters:
 * sender, recipient, amount, cardkey, location, date
 */
//void writeTxHttp()
//{
//	Serial.println("\nWriting HTTP...");
//	httpArgs="?";
//	httpArgs+= "sender=" + chipid;
//	httpArgs+= "&";
//	httpArgs+= "recipient=" + recipientAddress;
//	httpArgs+= "&";
//	httpArgs+= "amount=" "1";
//	httpArgs+= "&";
//	httpArgs+= "cardkey=" + cardKey;
//	httpArgs+= "&";
//	httpArgs+= "location=" + location;
//	httpArgs+= "&";
//	httpArgs+= "date=" + datetimenow();
//	Serial.println("HTTP DONE");
//}
/**
 * send tx to website
 */
void sendTx()
{
	HTTPClient http;
	String heroku_thumbprint;
	String httpTempData;
  String fullurl;
  fullurl = nodeurl;
	httpTempData="transactions/new"; //add card url
	//httpTempData+= httpArgs; //add card variable
	fullurl+= httpTempData;  //full url

	Serial.print("connecting to ");
	Serial.println(fullurl.c_str());
	http.begin(fullurl.c_str()); //connect to website
	http.addHeader("content-type","application/json"); //add http header, we set content to send in json
	int httpCode = http.POST(jsonArgs); //send the data to website

	//we get the http response code
	if(httpCode == HTTP_CODE_OK) {
		Serial.printf("httpcode: %d\n",httpCode);
		httpTempData = http.getString(); //receive data
		Serial.print("httpTempData");
		Serial.println(fullurl);
	}
	else {
		Serial.printf("error code: %d\n",httpCode);
	}
	http.end();
}
/**
 * shows the wifi ssid and ip address of the nodemcu
 */
String wifiInfo()
{
	String wifiinfostr;
	wifiinfostr="";
	wifiinfostr+="\n<br>Connected to ";
	wifiinfostr+=ssid;
	wifiinfostr+="\n<br>IP address: ";
	wifiinfostr+=chipip; //muestra ip asignada del nodemcu
	Serial.println(wifiinfostr);
	return wifiinfostr;
}
/**
 * connect to any wifi AP – returns true if successful or false if not
 */
boolean connectWifi()
{
	boolean state = true;
	int i = 0;

	WiFi.mode(WIFI_STA); //wifi mode is AP mode
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
	WiFi.begin(ssid, password);
	Serial.println("");
	Serial.println("Connecting to WiFi");

	// Wait for connection
	Serial.print("Connecting ...");
	while (WiFi.status() != WL_CONNECTED) { 
		delay(500);
		Serial.print(".");
		if (i > 10){
			state = false;
			break;
		}
		i++;
	}

	if (state){
		chipip = WiFi.localIP().toString();
		wifiInfo();
	}
	else {
		Serial.println("");
		Serial.println("Connection failed.");
	}
	return state;
}
  
/**
 * connect to default wifi – returns true if successful or false if not
 */
boolean connectWifiSetup()
{
	boolean state = true;
	int i = 0;

	WiFi.mode(WIFI_STA); //AP mode
	WiFi.begin(ssid, password); //connecting to defautl wifi
	Serial.println("");
	Serial.println("Connecting to WiFi");

	// Wait for connection
	Serial.print("Connecting ...");
	while (WiFi.status() != WL_CONNECTED) { //retry to connect multiple times
		delay(500);
		Serial.print(".");
		if (i > 10){
			state = false;
			break;
		}
		i++;
	}

	if (state){
		chipip = WiFi.localIP().toString();
		wifiInfo();
	}
	else {
		Serial.println("");
		Serial.println("Connection failed.");
	}
	return state;
}

/**
 * scan the rfid card in cardkey
 */
void scanRfidCard()
{
	Serial.println("\nPlace card in front of the RFID SCANNER");
	Serial.println("type exit to go back to menu");
	menuCmd="";
	//wait for card scan or select
	while ( (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial() ) && menuCmd != "exit") {
		readUserMenuCmd(); //scan a card or exit
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

		//SCAN THE CARD
		Serial.print("\nUID tag :");
		String cardKeyScanning= "";
		byte letter;
		for (byte i = 0; i < rfid.uid.size; i++)
		{
			Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : "-");
			Serial.print(rfid.uid.uidByte[i], HEX);
			cardKeyScanning.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : "-"));
			cardKeyScanning.concat(String(rfid.uid.uidByte[i], HEX));
		}
		cardKeyScanning.toUpperCase();
		Serial.println();
		rfid.PICC_HaltA();
		rfid.PCD_StopCrypto1();
		cardKey = cardKeyScanning;
	}
}

/**
 * show debug var, ssid, wifi pass, nodemcu location, cardkey, args
 */
void showDebug()
{
	Serial.println("WIFI VAR:");
	Serial.println(ssid);
	Serial.println(password);
	Serial.println(location);
	Serial.println("KEYCARD VAR:");
	Serial.println(cardKey);
	Serial.println(jsonArgs);
}
/**
 * str split from hmjd
 * https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
 */
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
