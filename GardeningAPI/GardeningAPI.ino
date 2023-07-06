#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "WifiCreds.h"

// WiFi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

// Pins Devices
const int Pumps = 33;

const int Sensor1 = 25;
const int Sensor2 = 26;
const int Sensor3 = 27;

// Other Constants
const int AutoMode = 1;
const int ManualMode = 0;

// TBD!
const int MinValue = 1; // Sensor Mapping
const int MaxValue = 100; // Sensor Mapping
const int Treshhold = 50; // Perentage when AutoMode will Water
const int WaterDelay = 10; // in s
const int LoopDelay = 1000; // in ms

Preferences pref;
StaticJsonDocument<250> doc;

int WateringCount, mode, WaterTime, DelayCount;
int V1, V2, V3;

WebServer server(80);  // create a server on port 80

void setup() {
  Serial.begin(115200);

  // Pin Control
  pinMode(Sensor1, INPUT);
  pinMode(Sensor2, INPUT);
  pinMode(Sensor2, INPUT);
  pinMode(Pumps, OUTPUT);

  //Preferences Safe
  pref.begin("ValueSafe", false);
  WateringCount = pref.getInt("WateringCount", 0);
  mode          = pref.getInt("mode", 0);
  WaterTime     = pref.getInt("WaterTime", 3000);
  V1            = pref.getInt("V1", 0);
  V2            = pref.getInt("V2", 0);
  V3            = pref.getInt("V3", 0);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Server Listening on:
  server.on("/", HTTP_GET, GetData);         // Get Current Water Level
  server.on("/", HTTP_POST, WaterControl);        // Control Watering in Manual Mode
  server.on("/mode", HTTP_GET, GetMode);     // Get current Mode
  server.on("/mode", HTTP_POST, SwitchMode); // Switch Mode between manual or auto
  server.begin();  // start the server
}


void loop() {
  // Read Sensor Data
  ReadSensor();
  // Check for incoming connections
  server.handleClient();
  // Auto Mode Implementation
  AutoWatering();

  //Delay
  delay(LoopDelay);
}


// HTTP Handler

// Send Sensor Data
void GetData(){

  doc["Sensor1"] = V1;
  doc["Sensor2"] = V2;
  doc["Sensor3"] = V3;

  String Body;
  serializeJson(doc, Body);

  server.send(200, "application/json", Body);
}


// Send current Mode
void GetMode(){
  doc["Mode"] = mode;

  String Body;
  serializeJson(doc, Body);

  server.send(200, "application/json", Body);  
}


//Switch between Manual and Auto Mode
void SwitchMode(){
  SetMode();
  server.send(200);
}


//Control Water in Manual Mode
void WaterControl(){

  if(mode = ManualMode){

    WaterGo();
    server.send(200);

  } else {
    server.send(403);
  }

}


// Functions

// Read all Sensors into Memory
void ReadSensor(){
  V1 = ValueMapping(analogRead(Sensor1));
  Serial.println(V1);
  V2 = ValueMapping(analogRead(Sensor2));
  Serial.println(V1);
  V3 = ValueMapping(analogRead(Sensor3));
  Serial.println(V1);
  
  pref.putInt("V1", V1);
  pref.putInt("V2", V2);
  pref.putInt("V3", V3);
}


//Set Pumps High for WaterTime
void WaterGo(){
  digitalWrite(Pumps, HIGH);
  delay(WaterTime);
  digitalWrite(Pumps, LOW);

  WateringCount++;
  pref.putInt("WateringCount", WateringCount);
}


//Switch between Manual and Auto
void SetMode(){
  if(mode = AutoMode){
    mode = ManualMode;
  } else if(mode = ManualMode){
    mode = AutoMode;
  } else {
    Serial.println("Error Mode State!");
    mode = AutoMode;
  }
  pref.putInt("mode", mode);
}


//Map Sensor Data to 1 - 100
int ValueMapping(int raw){
  int converted;
  converted = map(raw, MinValue, MaxValue, 0, 100);
  return(converted);
}


//Auto Watering if AutoMode is enabled
void AutoWatering(){
  DelayCount++;
  
  if(mode = AutoMode){
    if(V1 > Treshhold || V2 > Treshhold || V3 > Treshhold){
      if(DelayCount >= WaterDelay){

        WaterGo();
        DelayCount = 0;
      
      }
    }
  }

}

