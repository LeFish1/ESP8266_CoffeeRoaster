//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#include <FS.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <Ticker.h>

//#include <max6675.h>
#include <Wire.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <RunningMedian.h>

//needed for library
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <AsyncElegantOTA.h>

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

AsyncWebServer server(80);
DNSServer dns;

const char* ssid = "LeFish_2G";
const char* password = "wuaaiugi";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 11);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

int DimmerVal = 0;
int DimmerStep = 0;
float BeanTemp = 21.0;

struct DimmerVals
{
  int power = 0;  // Zeit, die Last eingeschalten ist.
  const int maxPower = 100;
  const int periodtime = 100; // Periodenzeit in ms.
  int currentTime = 0; // Zeit innerhalb der Periode in ms, wird bei Periodenanfang auf 0 zurückgesetzt.
  bool  isOn  = false; // Hält den Zustand des SSR
};

DimmerVals Dimmer;

//ESP8266 Pinout: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

const int SSR_on = D1;
const int hearbeatLed = D4;
bool heartbeatState = false;


/*
//Set Artisan Endpoints: 
//lt. https://github.com/esphome/ESPAsyncWebServer?utm_source=platformio&utm_medium=piohome#json-body-handling-with-arduinojson
// mit korrekter Syntax lt. https://github.com/me-no-dev/ESPAsyncWebServer/issues/594

AsyncCallbackJsonWebHandler* BeanTemp = new AsyncCallbackJsonWebHandler("/WebSocket/getBT", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonObject jsonBT_Req = json.as<JsonObject>();
  Serial.print("Command: ");
  Serial.println(jsonBT_Req["command"].as<char*>());
  Serial.print("ID: ");
  Serial.println(jsonBT_Req["id"].as<char*>());
  Serial.print("Machine: ");
  Serial.println(jsonBT_Req["machine"].as<char*>());
  //...
});
*/

WebSocketsServer webSocket = WebSocketsServer(8080);

void readDimmerVal()
{
  String SerialIn = "";
  while (Serial.available() > 0)
  {
    char incomingChar = Serial.read();
    SerialIn += incomingChar;
    delay(10);
  }

  if (SerialIn != "")
  {
    DimmerVal = SerialIn.toInt();
    Serial.print("New DimmerVal: ");
    Serial.println(DimmerVal);

  }

}

#define ESP8266

//===============================================
// Beantemp als Smoothed Thermocouple lt. https://github.com/YuriiSalimov/MAX6675_Thermocouple/blob/master/examples/SmoothMeasurement/SmoothMeasurement.ino
#define SCK_PIN D5
#define BeanTemp_CS_PIN D6
#define SO_PIN D7
/**
  Smoothing factor of a temperature value.
*/

Thermocouple* originThermocouple = NULL;
RunningMedian Med_BeanTemp = RunningMedian(12);


//MAX6675 Sensor_BT(SCK_PIN,BeanTemp_CS_PIN, SO_PIN);

//===============================================

void measureBeanTemp()
{

  //BeanTemp = SmoothBeanTemp->readCelsius();
  float this_BeanTemp = originThermocouple->readCelsius();
  Med_BeanTemp.add(this_BeanTemp);
  

  //DEBUG: Fake BeanTemp
  //BeanTemp = 111.2;

  //Serial.print("BeanTemp: ");
  //Serial.println(BeanTemp);

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  //Artisan schickt Anfrage als TXT
            //TXT zu JSON lt. https://forum.arduino.cc/t/assistance-parsing-and-reading-json-array-payload-websockets-solved/667917

  const size_t capacity = JSON_OBJECT_SIZE(3) + 60; //Memory pool
  DynamicJsonDocument doc(capacity);

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				
			        	// send message to client
				        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            { 
            //DEBUG WEBSOCKET
            //Serial.printf("[%u] get Text: %s\n", num, payload);

            //Extract Values lt. https://arduinojson.org/v6/example/http-client/
            //Artisan Anleitung: https://artisan-scope.org/devices/websockets/

            deserializeJson(doc, (char *)payload);

            //2 Nodes einzeln (Feld: "command"): "getBT" und "getDimmerVal"
            //oder
            // alle Nodes auf einmal (Feld: "command"): "getData"
            //char* entspricht String
            String command = doc["command"].as<char*>();
            long ln_id = doc["id"].as<long>();
            //Get BurnerVal from Artisan over Websocket
            if(!doc["BurnerVal"].isNull())
            {
              //Serial.print("BurnerVal: ");
              //Serial.println(doc["BurnerVal"].as<long>());  
              DimmerVal = doc["BurnerVal"].as<long>();
            }

            //Send Values to Artisan over Websocket
            JsonObject root = doc.to<JsonObject>();
            JsonObject data = root.createNestedObject("data");
            if(command == "getBT")
            {
              root["id"] = ln_id;
              data["BT"] = Med_BeanTemp.getMedian();
            }
            else if(command == "getDimmerVal")
            {
              root["id"] = ln_id;
              data["DimmerVal"] = float(DimmerVal);
            }
            else if(command == "getData")
            {
              root["id"] = ln_id;
              data["BT"] = Med_BeanTemp.getMedian();
              data["DimmerVal"] = float(DimmerVal);
            }

            //====================================
            //DEBUG
            /*
            if(!doc["command"].isNull())
            {
              Serial.print("Command: ");
              Serial.println(doc["command"].as<char*>());   
            }
            if(!doc["BurnerVal"].isNull())
            {
              Serial.print("BurnerVal: ");
              Serial.println(doc["BurnerVal"].as<long>());  
              DimmerVal = doc["BurnerVal"].as<long>();
            }
            
            Serial.print("ID: ");
            Serial.println(doc["id"].as<long>());
            Serial.print("RoasterID: ");
            Serial.println(doc["roasterID"].as<long>());

            //==========================
            //JETZT JSON-Payload generieren und senden!!!
            //==========================

            ln_id = doc["id"].as<long>();
            JsonObject root = doc.to<JsonObject>();
            JsonObject data = root.createNestedObject("data");
            root["id"] = ln_id;
            data["BT"] = BeanTemp;
            data["Dimmer"] = float(DimmerVal);

            */
           //DEBUG
           //====================================


            char buffer[200]; // create temp buffer
            size_t len = serializeJson(doc, buffer);  // serialize to buffer

            //DEBUG WEBSOCKET
            //Serial.println(buffer);

            webSocket.sendTXT(num, buffer);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            }
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}


// Ticker
void func1MS() {
    //Funktion zäehlt CurrentTime ++ und überprüft, ob SSR ein/aus. Am Ende einer Periode wird zurückgesetzt und die Temperatur ausgelesen.
    Dimmer.currentTime++;

    if (Dimmer.currentTime <= (Dimmer.power*Dimmer.periodtime/Dimmer.maxPower))
      Dimmer.isOn = true;
    else
      Dimmer.isOn = false;

    if (Dimmer.currentTime >= Dimmer.periodtime)
    {
      //Reset Period
      Dimmer.currentTime = 0;

      //Read new Dimmer Val
      //readDimmerVal();
      Dimmer.power = DimmerVal;
    }

    digitalWrite(SSR_on, !Dimmer.isOn); //Dimmer-Output ist Active-LOW
      
  }

  void heartbeat()
  {
    heartbeatState = !heartbeatState;
    digitalWrite(hearbeatLed,heartbeatState);
  }

  void func250MS() {
    measureBeanTemp();
  }

  void func1S() {
   heartbeat();
  }


Ticker ticker1MS(func1MS, 1, 0, MICROS);
Ticker ticker250MS(func250MS, 250, 0, MILLIS);
Ticker ticker1S(func1S, 1000, 0, MILLIS);

void setup() {

  Serial.begin(9600);

  pinMode(hearbeatLed, OUTPUT);

  //Dimmer
  pinMode(SSR_on, OUTPUT);
  Dimmer.isOn = false;
  digitalWrite(SSR_on, !Dimmer.isOn); //Dimmer-Output ist Active-LOW

  //Thermocouple
  originThermocouple = new MAX6675_Thermocouple(SCK_PIN, BeanTemp_CS_PIN, SO_PIN);

  ticker1MS.start();
  ticker250MS.start();
  ticker1S.start();

  /*

//SOFT AP MODE

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("CoffeRoaster_AP", "wuaaiugi") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  */
  

 /*

//CLIENT MODE

// We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  */

 
 //WIFI MANAGER

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  //AsyncWiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  //AsyncWiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);
  //AsyncWiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 32);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  AsyncWiFiManager wifiManager(&server,&dns);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,11), IPAddress(192,168,1,1), IPAddress(255,255,255,0));

  //add all your parameters here
  //wifiManager.addParameter(&custom_mqtt_server);
  //wifiManager.addParameter(&custom_mqtt_port);
  //wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("myWifi", "myPass", 1, 3000)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  

  //ELEGANT OTA

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Hi! I am ESP8266. Please connect to the page http://IP-Adress/update to perform an OTA-Update");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}

void loop() {
  webSocket.loop();
  ticker1MS.update();
  ticker250MS.update();
  ticker1S.update();
  AsyncElegantOTA.loop();
}