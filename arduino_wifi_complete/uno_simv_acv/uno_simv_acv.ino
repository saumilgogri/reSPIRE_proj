// ========================
//          SETUP
// ========================
#include <Guino.h>
//#include "Queue.h"

#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>


//====== Serial Connection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3

// setup servo
Servo servo;
int pos = 0;   

// ==== Analog Pins =====
int potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 5;
int pinMask = 4;
int pinDiff = 3;
int ledState = LOW;


// ==== Digital Pins =====
const int buzzerPin = 5;
const int ledPin = 6;
int inPin = 4;

// ==== Sensor Offset =====
float constPressureMask = 41;
float slopePressureMask = 92;
float constPressureDiff = 548;
float slopePressureDiff = 204.8;

// ===== Other vars ======
float check_value;
float IE_ratio;  
float TidVol;  
float BPM;
float separation;  
float sensorvalue;
float maskPressure;
float diffPressure;
float volFlow;
float diffPressureArr[90];
float maskPressureArr[90];
float area_1 = 0.0006158;
float area_2 = 0.000357;
float rho = 1.225;
//DynamicJsonDocument diffPressureJson(1024);
//Queue<float> diffPressureQ;
//Queue<float> maskPressureQ;


// ======= Mode changing vars =========
int state = HIGH;      
uint32_t lastPrint = millis();

void setup()
{
  servo.attach(9);  
  Serial.begin(9600);
  SUART.begin(9600); 
//  attachInterrupt(0, pin_ISR, CHANGE);
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
}

//////////////////
////// Loop //////
//////////////////
// ===========================================
//              RUN RESPIRATOR
// =========================================== 
void loop()
{
  if (digitalRead(inPin) == LOW)
  {while(digitalRead(inPin)==LOW) simv_mode();}
  
  else 
  {while(digitalRead(inPin) == HIGH) acv_mode();}

}
// ***************** END RUN RESPIRATOR  *******************

// ============ LAYER 1 FUNCTIONS ===============
// =================================================
//                 ACV MODE Function
// =================================================
void acv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;
  
  while(digitalRead(inPin) == HIGH)
  {
      // Fetch all potentiometer values
      fetchPotValues();
      // Initiate first run
      if(firstRun)
      {
        inspiration(TidVol);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio);
        firstRun = false;
      }
      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation || maskPressure < -1)
      {
        inspiration(TidVol);
        delay(15);
        cycleEndTime = expiration(TidVol, IE_ratio);
      }
      transmit(BPM, IE_ratio, maskPressure, TidVol, diffPressure);
      sanityCheckBuzzer();
      //for (int i = 0; i < sizeof(diffPressureArr); i++)                        
      //{
      //  diffPressureArr[i] = float(0);
      //  maskPressureArr[i] = float(0);
      //}
  }
  
}



// ===========================================
//            SIMV MODE Function
// ===========================================

void simv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(digitalRead(inPin) == LOW)
  {
      // Fetch all potentiometer values
      fetchPotValues();

      // ==== Initiate the cycle =====
      if(firstRun)
      {
        maskPressure = average_maskPressure();
        cycleEndTime = simv_logic(maskPressure, TidVol, IE_ratio);
        firstRun = false;
      }
      
      
      // ========= Identify trigger and initiate the cycle =============
      if(millis() - cycleEndTime >= (uint32_t)separation || average_maskPressure() < -1)
      {
        maskPressure = average_maskPressure();
        cycleEndTime = simv_logic(maskPressure, TidVol, IE_ratio);
      }
      transmit(BPM, IE_ratio, maskPressure, TidVol, diffPressure);
      

  }
}

// ============ LAYER 2 FUNCTIONS ================

// =======================
// SIMV Logic Function
// =======================

uint32_t simv_logic(float maskPressure, float TidVol, float IE_ratio)
{
    uint32_t cycleEndTime;
    maskPressure = int(floor(maskPressure));
    switch(int(maskPressure)){
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
// Average Pressure Function
// =======================

float average_maskPressure()
{
    maskPressure = 0;
    for(int i=0;i<5;i++)
    {
        maskPressure = maskPressure + pressureFromAnalog(pinMask, 1);
        delay(3);
    }
    return (maskPressure/5);
}


// =======================
// Inspiration Function
// =======================
void inspiration(float TidVol)
{ int count = 0;
  for(pos = 30; pos <= TidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servo.write(pos);
    delay(1000/TidVol);                       

    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);
    diffPressure = pressureFromAnalog(pinDiff, count);  
    volFlow = computeVolFlow();  
    Serial.println(volFlow);
    if (millis() - lastPrint >= uint32_t(100))
    { 
      //Serial.println(mean(diffPressure));
      lastPrint = millis();
    }
    count++;
  }
}

// =====================
// Expiration Function
// =====================

uint32_t expiration(float TidVol, float IE_ratio)
{
  int count = 0;
  for(int pos = TidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servo.write(pos);
    delay(1000*IE_ratio/TidVol);                       
    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);    
    diffPressure = pressureFromAnalog(pinDiff, count);   
    volFlow = computeVolFlow();  
    Serial.println(volFlow);
    if (millis() - lastPrint >= uint32_t(100))
    {
      //Serial.println(mean(diffPressure));
      lastPrint = millis();
    }
    count++;
  }  
  return millis();
}

// ======================================================
// Fetch Potentiometer values and adjust separation time
// ======================================================
void fetchPotValues()
{
  // Fetch all potentiometer values
      IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);    
      TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);     
      BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     
      separation = 1000*(60/BPM - (1+IE_ratio));  // convert to milliseconds
      // Correct separation time if needed
      if (separation < 0)
      { 
        IE_ratio = 60/BPM - 1;
        separation = (60/BPM - (1+IE_ratio))*1000;
      }
}

// =====================
// Transmit to DB
// =====================

void transmit(float BPM, float IE_ratio, float maskPressure, float TidVol, float diffPressure){
      String message = "";
      boolean messageReady = false;
      while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
      // The only messages we'll parse will be formatted in JSON
      DynamicJsonDocument doc(1024); // ArduinoJson version 6+
      // Attempt to deserialize the message
      DeserializationError error = deserializeJson(doc,message);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        messageReady = false;
        doc["type"] = "incorrect";
      }
      if(doc["type"] == "request") {
        doc["type"] = "response";
        doc["BPM"] = BPM;
        doc["IE_ratio"] = IE_ratio;
        doc["maskPressure"] = maskPressure;
        doc["TidVol"] = TidVol;
        doc["diffPressure"] = diffPressure; 
        serializeJson(doc,SUART);
      }
      }
      }

// ================= LAYER 3 FUNCTIONS =============
// =============================
// Pressure from Analog Function
// =============================
float pressureFromAnalog(int pin, int count)
{ float pressure;
  pressure = analogRead(pin);
  // Differential pressure sensor - output cmH2O
  if (pin == pinDiff)
  {
    pressure = (pressure - constPressureDiff)*1000/slopePressureDiff;  
    diffPressureArr[count] = pressure;
  }
  // Guage pressure sensor - output Pascal
  if (pin == pinMask)
  {
    pressure = (pressure - constPressureMask)*10.1972/slopePressureMask;
    maskPressureArr[count] = pressure;
  }

  return pressure;
}


// =============================
// Alarm Sanity Check
// =============================

void sanityCheckBuzzer()
{
  float SDPressure;
  SDPressure = calcSD(maskPressureArr);
  //Serial.println(SDPressure);
  if(SDPressure < 0.01) buzzAlarm(true);
  if(SDPressure >= 0.01) buzzAlarm(true);
}

// ================= LAYER 4 FUNCTIONS =============

// =============================
// Calculate standard deviation
// =============================
float calcSD(float data[])
{
  float avg = 0.0, SD = 0.0;
  int length = sizeof(data);

  for (int i = 0; i < length; i++)
  {
    avg += data[i];
  }
  avg = avg/length;

  for (int i = 0; i < length; i++)
  {SD += pow(data[i] - avg, 2);}
  return sqrt(SD/length);
}

// ================================================================
// Buzz Alarm if there is a problem; stop alarm if problem resolved
// ================================================================
void buzzAlarm(bool turnOn)
{
  if (turnOn == true)
  {
    ledState = HIGH;
    tone(buzzerPin,500);
  }

  if (turnOn == false)
  {
    ledState = LOW;
    noTone(buzzerPin);
  }
  
}

// =======================
// Copmute Vol Flow
// =======================
float computeVolFlow()
{ 
  return 1000*sqrt((abs(diffPressure)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2)))))/rho; 
}