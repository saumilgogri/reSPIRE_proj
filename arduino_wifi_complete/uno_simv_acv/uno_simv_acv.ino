// ========================
//          SETUP
// ========================
#include <Guino.h>
//#include "Queue.h"

#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Nextion.h>
#include "NexButton.h"

//====== Serial Connection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3
SoftwareSerial nextion(0,1);
// setup servo
Servo servo;
int pos = 0;   
//Nextion myNextion(nextion,9600);
//Nextion myNextion(nextion, 9600); //create a Nextion object named myNextion using the nextion serial port @ 9600bps

// ==== Analog Pins =====
int potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 2;
int pinMask = 4;
int pinDiff = 3;
int ledState = LOW;


// ==== Digital Pins =====
const int buzzerPin = 5;
const int ledPin = 6;

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
float area_1 = 0.0002835287370;
float area_2 = 0.00007853981634;
float rho = 1.225;
float totVolume;
float timeNow;
String acvLabel = "acv";
String simvLabel = "simv";

// ======= Mode changing vars =========
int state = HIGH;      
uint32_t lastPrint = millis();

// Declare your Nextion objects - Example (page id = 0, component id = 1, component name = "b0") 
NexText t_mode = NexText(0, 9, "t_mode");
NexButton b0 = NexButton(0, 2, "b0");
NexButton b1 = NexButton(0, 3, "b1");
NexButton b2 = NexButton(0, 4, "b2");
NexText t3 = NexText(1, 5, "t3");
NexText t4 = NexText(1, 6, "t4");
NexText t5 = NexText(1, 7, "t5");
NexText t_ie_ratio = NexText(0, 10, "t_ie_Ratio");
NexText t_bpm = NexText(0, 11, "t_bpm");
NexText t_tidvol = NexText(0, 12, "t_tidvol");
String set_mode;
NexTouch *nex_listen_list[] = {&b0,&b1,&b2,NULL};


void setup()
{
  servo.attach(9);  
  Serial.begin(9600);
  SUART.begin(9600); 
//  attachInterrupt(0, pin_ISR, CHANGE);
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);

  // Register the pop event callback function of the components
  nexInit();
  //myNextion.init();
  b0.attachPop(b0PopCallback, &b0);
  b1.attachPop(b1PopCallback, &b1);
  b2.attachPop(b2PopCallback, &b2);
}

//////////////////
////// Loop //////
//////////////////
// ===========================================
//              RUN RESPIRATOR
// =========================================== 
void loop()
{
  //acv_mode();
  //while(set_mode == simvLabel) simv_mode();
//  get b0.val;
  //while(set_mode == acvLabel) acv_mode();

//maskPressure = pressureFromAnalog(pinMask,1000);
//diffPressure = pressureFromAnalog(pinDiff,1000); 
//fetchPotValues();

nexLoop(nex_listen_list);
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
  
  while(true)
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
      sanityCheckBuzzer();
       
      // ============ Update pressure values =========
      maskPressure = pressureFromAnalog(pinMask,1000);
      diffPressure = pressureFromAnalog(pinDiff,1000); 
      Serial.println(maskPressure);
      //print_to_screen();
      //nexLoop(nex_listen_list);
      //computePrintVolFlow();
  }
  return;
}



// ===========================================
//            SIMV MODE Function
// ===========================================

void simv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(set_mode = simvLabel)
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
  }
  return;
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
  totVolume = 0;
  timeNow = millis();
  for(pos = 30; pos <= TidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    
    servo.write(pos);
    delay(1000/TidVol);                       

    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);
    diffPressure = pressureFromAnalog(pinDiff, count);  
    computePrintVolFlow();  
    Serial.println(maskPressure);
    //nexLoop(nex_listen_list); 
    //print_to_screen();
    count++;
  }
}

// =====================
// Expiration Function
// =====================

uint32_t expiration(float TidVol, float IE_ratio)
{
  int count = 0;
  totVolume = 0;
  timeNow = millis();
  for(int pos = TidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                               
    servo.write(pos);
    delay(1000*IE_ratio/TidVol);                       
    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);    
    diffPressure = pressureFromAnalog(pinDiff, count);   
    Serial.println(maskPressure);
    //computePrintVolFlow();
    //nexLoop(nex_listen_list); 
    //print_to_screen();
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
    if (count != 1000) {diffPressureArr[count] = pressure;}
  }
  // Guage pressure sensor - output Pascal
  if (pin == pinMask)
  {
    pressure = pressure = (pressure - constPressureMask)*10.1972/slopePressureMask;;
    if (count != 1000) {maskPressureArr[count] = pressure;}
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
void computePrintVolFlow()
{ 
  float flow;
  flow =  1000*sqrt((abs(diffPressure)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2)))))/rho; 
  if (millis() - lastPrint >= uint32_t(100))
    { 
      //Serial.println(flow);
      lastPrint = millis();
    }
  if(flow > 0.4)totVolume = totVolume + flow*(millis() - timeNow);
  timeNow = millis();
  //Serial.println(totVolume);
}
// =======================
// Nextion Screen Functions
// =======================
void b0PopCallback(void *ptr) {
  set_mode = "MODE : ACV"; 
  t_mode.setText(set_mode.c_str());
}

void b1PopCallback(void *ptr) {
  set_mode = "MODE : SIMV"; 
  t_mode.setText(set_mode.c_str());  
}

void b2PopCallback(void *ptr) {
  set_mode = "MODE : None"; 
  t_mode.setText(set_mode.c_str());
}

// =====================
// Print to Screen
// =====================
void print_to_screen()
{ 
  t_mode.setText(set_mode.c_str());
  static char ps_diff[6];
  dtostrf(diffPressure, 6, 2, ps_diff);
  static char ps_mask[6];
  dtostrf(maskPressure, 6, 2, ps_mask);
  t3.setText(ps_diff);
  t5.setText(ps_mask);
  static char ie_ratio[6];
  dtostrf(IE_ratio, 6, 2, ie_ratio);
  t_ie_ratio.setText(ie_ratio);
  static char bpm[6];
  dtostrf(BPM, 6, 2, bpm);
  t_bpm.setText(bpm);
  static char tidvol[6];
  dtostrf(TidVol, 6, 2, tidvol);
  t_tidvol.setText(tidvol);
  
  
}
