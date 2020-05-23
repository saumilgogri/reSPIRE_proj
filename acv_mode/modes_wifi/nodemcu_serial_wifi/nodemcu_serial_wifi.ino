
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif



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
float list_values[5] ;

//=======================================
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

boolean newData = false;


// Replace with your network credentials
const char* ssid     = "NETGEAR43";
const char* password = "smilingnest196";  

// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://respire.000webhostapp.com/post-esp-data.php";

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "xPmAT5Ab3j7F6";

String sensorName = "MPXDIFF";
String sensorLocation = "San Diego";

void setup() {
  Serial.begin(115200); //enable Serial Monitor
  SUART.begin(115200); //enable SUART Port
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
  recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        newData = false;
    }
  Serial.print(list_values[0]);
  send_data(list_values);
  
  }

//======================================

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//========================================

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index

    for (int i=0;i<5;i++)
  {
    if(i=0){
      strtokIndx = strtok(tempChars,",");
    }
    else
    {
      strtokIndx = strtok(NULL, ",");
    }
    list_values[i] = atof(strtokIndx);
  }
}

void send_data(float list_data[]) {
if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Prepare your HTTP POST request data
    //sample1 = sample1 + 0.646;
    //sample2 = sample2 + 0.545;
    //sample3 = sample3 + 0.666;
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(list_data[0])
                          + "&value2=" + String(list_data[1]) + "&value3=" + String(list_data[2]) + "";
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
