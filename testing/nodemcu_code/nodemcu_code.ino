
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
#include <FirebaseArduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include<SoftwareSerial.h>
SoftwareSerial SUART(4, 5); //SRX=Dpin-D2; STX-DPin-D1

#define FIREBASE_HOST "respire-447da.firebaseio.com"
#define FIREBASE_AUTH "vsloiSErdIEyQMV6ydzwaELOPVSP5dPqiOobAURm"
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "europe.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

//=======================================
unsigned long time_1;
bool test_2;
String last_char = "]";
time_t start_time;
//const byte numChars = 300;
//char receivedChars[numChars];   // an array to store the received data

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

boolean newData = false;

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
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.setFloat("test_number", 42.0); 
  Serial.println("Connected to Firebase");
  
  timeClient.begin();
}

void loop() {
  timeClient.update();
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonBuffer<capacity> jb;
  JsonObject& doc = jb.createObject();
  doc["type"] = "request";
  String message = "";
  String test = "";
  boolean messageReady = false;
  doc.printTo(SUART);
  start_time = millis();
  message = SUART.readString();
  send_data(message);
  jb.clear();
  delay(3000);
  }
  

//======================================


void send_data(String message) {
if(WiFi.status()== WL_CONNECTED){
    String time_now = timeClient.getFormattedTime();
    Firebase.setString(time_now, message);
    Serial.println(message);
    Serial.println("The string was pushed");
}}
