
#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
#include "Nextion.h"

//====== Serial COnnection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3

Servo servoright;
int pos = 0;   

// ==== Analog Pins =====
float potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 5;
int pinguage_mask = 4;
int pinguage_expiration = 3;
int pinguage_diff = 2;

// ==== Analog Pins ====
int inPin = 4;

// ==== Sensor Offset =====
const float guageSensorOffset = 41;
const float pressureDiffSensorOffset = 41;

// ===== Other vars ======
float check_value;
float IE_ratio;  
float TidVol;  
float BPM;
float separation;  
float sensorvalue;
float pressure_mask;
float pressure_expiration;
float pressure_diff;

int display_IE_ratio;
int display_BPM;
int display_TidVol;
int display_sensorvalue_cmh2o;
int display_sensorvalue_gp_cmh2o;

// ======= Mode changing vars =========

int state = HIGH;      // the current state of the output pin
int mode;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin
long t_time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


// ======= JSON For storing data =========
String message = "";
boolean messageReady = false;
int obj_number = 1;
const int send_doc_capacity = JSON_ARRAY_SIZE(10) + 2*JSON_OBJECT_SIZE(7);
StaticJsonBuffer<send_doc_capacity> send_doc_json_buffer;
JsonArray& arr = send_doc_json_buffer.createArray();

void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  SUART.begin(9600); //enable SUART Port for communication with NODEMCU
//  attachInterrupt(0, pin_ISR, CHANGE);
}
 
void loop()
{
  if (digitalRead(inPin) == LOW)
  {
      while(digitalRead(inPin)==LOW)
      {
        simv_mode();
        while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
        transmit(message);
      }
  }
  }
  else 
  {   while(digitalRead(inPin) == HIGH)
      {
        acv_mode();
        while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
        transmit(message);
      }  
      }
      
  }

}

// =======================
// ACV MODE Function
// =======================

void acv_mode()
{
  float IE_ratio;
  float TidVol;
  float BPM;
  float per_breath_time;
  float per_inspiration_time;
  float per_expiration_time;
  float human_effort;
  float separation;
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(digitalRead(inPin) == HIGH)
  {
      // Fetch all potentiometer values
      IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);    
      TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);     
      BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     
      separation = (60/BPM - (1+IE_ratio));
      if (separation < 0)
      { IE_ratio = 60/BPM - 1;
        separation = 60/BPM - (1+IE_ratio);
       }

      // Fetch pressure sensor values
      pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
      pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
      pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
      // Initiate the cycle
      if(firstRun)
      {
        inspiration(TidVol);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio);
        firstRun = false;
      }

      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation*1000 || pressure_mask < -1)
      {
        inspiration(TidVol);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio);
      }
      make_json(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
  }
  
}

// =======================
// SIMV Logic Function
// =======================

uint32_t simv_logic(float pressure_mask, float TidVol, float IE_ratio)
{
    uint32_t cycleEndTime;
    pressure_mask = int(floor(pressure_mask));
    switch(int(pressure_mask)){
        case -6:
        case -5:
        case -4:
            TidVol = TidVol*0.25;
            inspiration(TidVol);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio);
            break;
        case -3:
            TidVol = TidVol*0.50;
            inspiration(TidVol);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio);
            break;
        case -2:
            TidVol = TidVol*0.75;
            inspiration(TidVol);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio);
            break;
        case -1:
            TidVol = TidVol*0.75;
            inspiration(TidVol);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio);
            break;
        default:
            inspiration(TidVol);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio);
            break;
    }
    return cycleEndTime;
}

// =======================
// SIMV MODE Function
// =======================

void simv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(digitalRead(inPin) == LOW)
  {
      // Fetch all potentiometer values
      IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);    
      TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);     
      BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     
      separation = (60/BPM - (1+IE_ratio));
      if (separation < 0)
    { IE_ratio = 60/BPM - 1;
        separation = 60/BPM - (1+IE_ratio);}
      // Fetch pressure sensor values
      pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
      pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
      pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
      
      // Initiate the cycle
      if(firstRun)
      {
        pressure_mask = average_pressure_mask();
        cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio);
        firstRun = false;
      }

      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation*1000 || average_pressure_mask() < -1)
      {
        pressure_mask = average_pressure_mask();
        cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio);
      }
      make_json(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
  }
}

// =======================
// Average Pressure Function
// =======================

float average_pressure_mask()
{
    pressure_mask = 0;
    for(int i=0;i<5;i++)
    {
        pressure_mask = pressure_mask + (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
        delay(3);
    }
    return (pressure_mask/5);
}


// =======================
// Inspiration Function
// =======================
void inspiration(float TidVol)
{ int pos;
  for(pos = 30; pos <= TidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servoright.write(pos);
    delay(1000/TidVol);                       

    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
    Serial.println(pressure_mask);
    //Serial.println(pressure_diff);
    //Serial.println(pressure_expiration);
  }

}

// =====================
// Expiration Function
// =====================

uint32_t expiration(float TidVol, float IE_ratio)
{
  for(pos = TidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servoright.write(pos);
    delay(1000*IE_ratio/TidVol);                       
    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
    Serial.println(pressure_mask);
    //Serial.println(pressure_diff);
    //Serial.println(pressure_expiration);
  }  
  return millis();
}

// =====================
// Transmit to DB
// =====================

void transmit(String message){
      // The only messages we'll parse will be formatted in JSON
      const int capacity = JSON_OBJECT_SIZE(2);
      StaticJsonBuffer<capacity> jb;
      JsonObject& doc = jb.parseObject(message);
      //DynamicJsonDocument doc(1024); // ArduinoJson version 6+
      // Attempt to deserialize the message
      if(!(doc.success())){
        Serial.println("Doc conversion failed");
        messageReady = false;
        doc["type"] = "incorrect";
      }
      if(doc["type"] == "request") { 
        arr.printTo(SUART);
        send_doc_json_buffer.clear();
      }
      }



// =====================
// Make the JSON
// =====================

void make_json(float BPM, float IE_ratio, float pressure_mask, float pressure_expiration, float TidVol, float pressure_diff){
  JsonObject& obj = send_doc_json_buffer.createObject();
  obj["type"] = "response";
  obj["BPM"] = BPM;
  obj["IE_ratio"] = IE_ratio;
  obj["pressure_mask"] = pressure_mask;
  obj["pressure_expiration"] = pressure_expiration;
  obj["TidVol"] = TidVol;
  obj["pressure_diff"] = pressure_diff;
  arr.add(obj);
  obj.remove("type");
  obj.remove("TidVOl");
  obj.remove("pressure_expiration");
  obj.remove("pressure_mask");
  obj.remove("IE_ratio");
  obj.remove("BPM");
  
}

