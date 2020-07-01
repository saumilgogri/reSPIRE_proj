#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <Nextion.h>


//======== Information about SD Card ====
/*
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 */
 
//====== Serial COnnection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3

//====== LCD Variables ==============
int id_1 = 1;
int ch = 0;
int id_2 = 8;
int id_3 = 9;
String ad = "add ";
char buffer[100] = {0};
char buffer_2[100] = {0};
char buffer_3[100] = {0};
NexText t0 = NexText(0, 2, "t0");
NexText t1 = NexText(0, 3, "t0");
NexText t2 = NexText(0, 4, "t2");

//===== Variables =====
Servo servoright;
int pos = 0;   

//======= SD Card File ===========
File ventilator_data;

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
int previous = LOW;    // the previous reading from the input pin
long t_time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers

String mode = "";


void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  SUART.begin(9600); //enable SUART Port for communication with NODEMCU
  pinMode(10,OUTPUT);
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("SD Card Initialization done.");
  ventilator_data = SD.open("ventilator_data.txt",FILE_WRITE);
  
}
 
void loop()
 
{
  if (digitalRead(inPin) == LOW)
  {
      while(digitalRead(inPin)==LOW)
      {
        simv_mode();
      }
  }
  else 
  {   while(digitalRead(inPin) == HIGH)
      {
        acv_mode();  
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
  mode = "ACV";

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
        inspiration(TidVol, mode);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio, mode);
        firstRun = false;
      }

      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation*1000 || pressure_mask < -1)
      {
        inspiration(TidVol, mode);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio, mode);
      }
      transmit(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
      send_to_screen(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
  }
  
}

// =======================
// SIMV Logic Function
// =======================

uint32_t simv_logic(float pressure_mask, float TidVol, float IE_ratio, String mode)
{
    uint32_t cycleEndTime;
    pressure_mask = int(floor(pressure_mask));
    switch(int(pressure_mask)){
        case -6:
        case -5:
        case -4:
            TidVol = TidVol*0.25;
            inspiration(TidVol, mode);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio, mode);
            break;
        case -3:
            TidVol = TidVol*0.50;
            inspiration(TidVol, mode);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio, mode);
            break;
        case -2:
            TidVol = TidVol*0.75;
            inspiration(TidVol, mode);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio, mode);
            break;
        case -1:
            TidVol = TidVol*0.75;
            inspiration(TidVol, mode);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio, mode);
            break;
        default:
            inspiration(TidVol, mode);
            delay(15);
            cycleEndTime = expiration(TidVol, IE_ratio, mode);
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
  mode = "SIMV";

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
        cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio, mode);
        firstRun = false;
      }

      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation*1000 || average_pressure_mask() < -1)
      {
        pressure_mask = average_pressure_mask();
        cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio, mode);
      }
      transmit(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
      send_to_screen(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
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
void inspiration(float TidVol, String mode)
{ int pos;
  for(pos = 30; pos <= TidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servoright.write(pos);
    delay(1000/TidVol);                       

    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    String data = mode + "," + pressure_mask + "," + pressure_diff + "," + pressure_expiration + ";";
    ventilator_data.println(data);
  }

}

// =====================
// Expiration Function
// =====================

uint32_t expiration(float TidVol, float IE_ratio, String mode)
{
  for(pos = TidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servoright.write(pos);
    delay(1000*IE_ratio/TidVol);                       
    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    String data = mode + "," + pressure_mask + "," + pressure_diff + "," + pressure_expiration + ";";
    ventilator_data.println(data);
  }  
  return millis();
}

// =====================
// Transmit to DB
// =====================

void transmit(float BPM, float IE_ratio, float pressure_mask, float pressure_expiration, float TidVol, float pressure_diff){
      String message = "";
      boolean messageReady = false;
      while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
      // The only messages we'll parse will be formatted in JSON
      const int capacity = JSON_OBJECT_SIZE(7);
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
        doc["type"] = "response";
        doc["BPM"] = BPM;
        doc["IE_ratio"] = IE_ratio;
        doc["pressure_mask"] = pressure_mask;
        doc["pressure_expiration"] = pressure_expiration;
        doc["TidVol"] = TidVol;
        doc["pressure_diff"] = pressure_diff; 
        doc.printTo(SUART);
      }
      }
      }

// =====================
// Print to Screen
// =====================

void send_to_screen(float BPM, float IE_ratio, float pressure_mask, float pressure_expiration, float TidVol, float pressure_diff){
  float p_mask = map(pressure_mask, 5.00, 20.00, 0, 255);
  float p_expiration = map(pressure_mask, 5.00, 20.00, 0, 255);
  float p_diff = map(pressure_mask, 5.00, 20.00, 0, 255);
  String to_send_p_mask = ad + id_1 + "," + ch + "," + int(p_mask);
  print_screen(to_send_p_mask);
  String to_send_p_diff = ad + id_2 + "," + ch + "," + int(p_diff);
  print_screen(to_send_p_diff);
  String to_send_p_exp = ad + id_3 + "," + ch + "," + int(p_expiration);
  print_screen(to_send_p_exp);
  dtostrf(BPM, 6, 2, buffer);
  t0.setText(buffer);
  dtostrf(IE_ratio,6,2,buffer_2);
  t1.setText(buffer_2);
  dtostrf(TidVol,6,2,buffer_3);
  t2.setText(buffer_3);
}

void print_screen(String to_send){
  Serial.print(to_send);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

