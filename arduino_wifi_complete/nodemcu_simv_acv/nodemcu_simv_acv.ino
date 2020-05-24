
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif
#include <ArduinoJson.h>
#include <Wire.h>

#include<SoftwareSerial.h>
SoftwareSerial SUART(4, 5); //SRX=Dpin-D2; STX-DPin-D1


//===================================
//  Variables for storing pressure data
//===================================

float BPM = 0.00;
float IE_ratio = 0.00;
float TidVol = 0.00;
float pressure_mask = 0.00;
float pressure_diff = 0.00;
float pressure_expiration = 0.00;

//=======================================
unsigned long time_1;
bool test_2;
String last_char = "}";


// Replace with your network credentials
const char* ssid     = "1.21 Jiggawatts!";
const char* password = "itburnswhenIP";  

// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://respire.000webhostapp.com/post-esp-data.php";

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "xPmAT5Ab3j7F6";

String sensorName = "MPXDIFF";
String sensorLocation = "San Diego";

void setup() {
  Serial.begin(9600); //enable Serial Monitor
  SUART.begin(9600); //enable SUART Port
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
}

void loop() {
  String message = "";
  //String test = "";
  DynamicJsonDocument doc(1024);
  doc["type"] = "request";
  //serializeJson(doc, test);
  //Serial.println(test);
  serializeJson(doc,SUART);
  // Reading the response
  boolean messageReady = false;
  time_1 = millis();
  while(messageReady == false) { // blocking but that's ok
    if(SUART.available()) {
      message = SUART.readString();
      Serial.println(message);
      test_2 = message.endsWith(last_char);
      Serial.println(test_2);
      if (!(test_2)){
        return;
      }
      Serial.println("I am here");
      messageReady = true;
    }

  }
  send_data(message);
  delay(3000);
  }
  

//======================================


void send_data(String message) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc,message);
  if(error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String BPM = doc["BPM"];
    String IE_ratio = doc["IE_ratio"];
    String TidVol = doc["TidVol"];
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + BPM
                          + "&value2=" + IE_ratio + "&value3=" + TidVol + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
  
  int httpResponseCode = http.POST(httpRequestData);
  
  if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
}}
