// ========================
//          SETUP
// ========================
#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
#include <Nextion.h>
#include <SPI.h>
#include <SD.h>

//====== Serial Connection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3

// setup servo
Servo servo;
float pos = 0;   

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
float constPressureMask = 512+24.1;
float slopePressureMask = 204.8;
float constPressureDiff = 512+21;
float slopePressureDiff = 204.8;

// ===== Other vars ======
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
//====== LCD Variables ==============
int start = 0; 
int id_1 = 4;
int ch = 0;
int id_2 = 3;
int id_3 = 2;
String rawMsg, msg, pageNum;
String ad = "add ";
static char buffer_1[10] = {};
static char buffer_2[10] = {};
static char buffer_3[10] = {};
static char buffer_mode[10] = {};
String set_mode = "";
NexText IERatioText_Page0 = NexText(0, 13, "t_ie_ratio");
NexText BPMText_Page0 = NexText(0, 14, "t_bpm");
NexText TidVolText_Page0 = NexText(0, 15, "t_tidvol");
NexText IERatioText_Page1 = NexText(1, 11, "t_ie_ratio");
NexText BPMText_Page1 = NexText(1, 12, "t_bpm");
NexText TidVolText_Page1 = NexText(1, 13, "t_tidvol");

NexText t_mode = NexText(0, 4, "t_mode");
NexButton SIMVButton = NexButton(0, 2, "SIMVButton");
NexButton ACVButton = NexButton(0, 1, "ACVButton");
NexButton PSVButton = NexButton(0, 9, "PSVButton");
NexButton NoModeButton = NexButton(0, 10, "NoModeButton");
NexButton startButton = NexButton(0, 11, "startButton");
NexButton stopButton = NexButton(1, 1, "stopButton");
NexTouch *nex_listen_list[] = {&SIMVButton, &ACVButton, &PSVButton, &NoModeButton, &startButton, &stopButton, NULL};

/* NexText t_mode = NexText(0, 9, "t_mode");
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
NexTouch *nex_listen_list[] = {&b0,&b1,&b2,NULL}; */


void SIMVButtonPopCallback(void *ptr) {
  set_mode = "SIMV";
  String data = "page0.t_mode.txt=\"" + set_mode  + "\"";writeString(data);
}

void ACVButtonPopCallback(void *ptr) {
  set_mode = "ACV";
  String data = "page0.t_mode.txt=\"" + set_mode  + "\"";writeString(data);
}
void PSVButtonPopCallback(void *ptr) {
  set_mode = "PSV";
  String data = "page0.t_mode.txt=\"" + set_mode  + "\"";writeString(data);
}
void NoModeButtonPopCallback(void *ptr) {
  set_mode = "NoMode";
  String data = "page0.t_mode.txt=\"" + set_mode  + "\"";writeString(data);
}
void startButtonPopCallback(void *ptr) {
  start = 1;

}
void stopButtonPopCallback(void *ptr) {
  start = 0;
}

//======= SD Card File ===========
File myFile;

void setup()
{
  servo.attach(9);  
  Serial.begin(9600);
  SUART.begin(9600); 
  pinMode(ledPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  pinMode(10, OUTPUT);
  nexInit();
  SIMVButton.attachPop(SIMVButtonPopCallback, &SIMVButton);
  ACVButton.attachPop(ACVButtonPopCallback, &ACVButton);
  PSVButton.attachPop(PSVButtonPopCallback, &PSVButton);
  NoModeButton.attachPop(NoModeButtonPopCallback, &NoModeButton);
  startButton.attachPop(startButtonPopCallback, &startButton);
  stopButton.attachPop(stopButtonPopCallback, &stopButton);

  //if (!SD.begin(4)) {
  // Serial.println("initialization failed!");
  // return;
  //}
  //Serial.println("SD Card Initialization done.");
  /* myFile = SD.open("example.txt", FILE_WRITE);
  myFile.close(); */
}
void writeString(String stringData){
//Function to send commands to the Nextion display.
  for(int i=0; i < stringData.length(); i++){Serial.write(stringData[i]);}
  Serial.write(0xff);Serial.write(0xff);Serial.write(0xff);
}
//////////////////
////// Loop //////
//////////////////
// ===========================================
//              RUN RESPIRATOR
// =========================================== 
void loop()
{


BPM = 20;
IE_ratio = 2;
TidVol = 15;
maskPressure = 2;
volFlow = 4;
totVolume = 800;

transmit();
delay(3000);


/*     fetchPotValues();
    send_to_screen_values();
    delay(500);
    nexLoop(nex_listen_list);
    if(set_mode == "ACV" && start == 1)
    {
      Serial.println("I am here");
      simv_mode();    
    }
    Serial.println("I am not lost!!"); */
    //acv_mode();
    //Serial.println(set_mode);
    //Serial.println(start);
  //if(set_mode == "ACV" && start == true)
  //{
  //    acv_mode();
  //}
  
  //send_to_screen_values();
  //}
  
  //delay(500);
  //send_to_screen_values();

  /*   send_to_screen_values();
    if (set_mode == "ACV") {
    //myFile = SD.open("example.txt", FILE_WRITE);
    acv_mode();
    //myFile.close();
  }
  else if (set_mode == "SIMV") {
    //myFile = SD.open("example.txt", FILE_WRITE);
    simv_mode();
    //myFile.close();
  } */
  
}
// ***************** END RUN RESPIRATOR  *******************

// ============ LAYER 1 FUNCTIONS ===============
// =================================================
//                 SIMV MODE Function
// =================================================
void simv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(true)
  {
      // Fetch all potentiometer values
      fetchPotValues();

      // ==== Initiate the cycle =====
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

      maskPressure = pressureFromAnalog(pinMask,1000);
      diffPressure = pressureFromAnalog(pinDiff,1000); 
      Serial.println(totVolume);
  }
  return;
}



// ===========================================
//            ACV MODE Function
// ===========================================

void acv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;

  while(true)
  {
      // Fetch all potentiometer values
      fetchPotValues();

      // ==== Initiate the cycle =====
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

      maskPressure = pressureFromAnalog(pinMask,1000);
      diffPressure = pressureFromAnalog(pinDiff,1000); 
      Serial.println(IE_ratio);
  }
  return;
}

// ============ LAYER 2 FUNCTIONS ================

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
  for(pos = 0; pos <= TidVol; pos += 0.5) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    
    servo.write(pos+1.5);
    delay(1000/TidVol);                       

    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);
    diffPressure = pressureFromAnalog(pinDiff, count);  
    computePrintVolFlow();  
    Serial.println(IE_ratio);
    //String data = set_mode + "," + String(maskPressure) + "," + String(volFlow) + "," + String(totVolume) + ";";
    //Serial.println(set_mode + "," + String(maskPressure) + "," + String(volFlow) + "," + String(totVolume) + ";");
    //myFile.println(data);
    //nexLoop(nex_listen_list);
    //send_to_screen_values();
    //nexLoop(nex_listen_list); 
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
  for(int pos = TidVol; pos>=2; pos-=0.5)     // goes from 180 degrees to 0 degrees
  {                               
    servo.write(pos-1.5);
    delay(1000*IE_ratio/TidVol);                       
    // ============ Update pressure values =========
    maskPressure = pressureFromAnalog(pinMask, count);    
    diffPressure = pressureFromAnalog(pinDiff, count);   
    //computePrintVolFlow();
    //String data = set_mode + "," + String(maskPressure) + "," + String(volFlow) + "," + String(totVolume) + ";";
    //Serial.println(set_mode + "," + String(maskPressure) + "," + String(volFlow) + "," + String(totVolume) + ";");
    //myFile.println(data);
    Serial.println(IE_ratio);
    //send_to_screen_values();
    //nexLoop(nex_listen_list); 
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
      TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 120.00);     
      BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     
      separation = 1000*(60/BPM - (1+IE_ratio));  // convert to milliseconds
      // Correct separation time if needed
      if (separation < 0)
      { 
        IE_ratio = 60/BPM - 1;
        separation = (60/BPM - (1+IE_ratio))*1000;
      }
}


// ================= LAYER 3 FUNCTIONS =============
// =============================
// Pressure from Analog Function
// =============================
float pressureFromAnalog(int pin, int count)
{ float pressure;
  pressure = analogRead(pin);
  // Differential pressure sensor - output Pascal
  if (pin == pinDiff)
  {
    pressure = (pressure - constPressureDiff)*1000/slopePressureDiff;  
    if (count != 1000) {diffPressureArr[count] = pressure;}
  }
  // Guage pressure sensor - output cmH20
  if (pin == pinMask)
  {
    pressure = (pressure - constPressureMask)*10.1972/slopePressureMask;;
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
  //Serial.println(SDPressure);
  if(SDPressure < 0.01) buzzAlarm(true);
  if(SDPressure >= 0.01) buzzAlarm(false);
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
  volFlow =  1000*sqrt((abs(diffPressure)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2)))))/rho; 
  if (millis() - lastPrint >= uint32_t(100))
    { 
      //Serial.println(flow);
      lastPrint = millis();
    }
  if(volFlow > 0.4)totVolume = totVolume + volFlow*(millis() - timeNow);
  timeNow = millis();
  //Serial.println(totVolume);
}
// =======================
// Nextion Screen Functions
// =======================
/* void b0PopCallback(void *ptr) {
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
} */

// =====================
// Print to Screen
// =====================
/* void print_to_screen()
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
  
  
} */


// =====================
// Transmit to DB
// =====================
void transmit(){
      Serial.println("Inside the transmit function");
      String message = "";
      boolean messageReady = false;
      while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
      const int capacity_data = JSON_OBJECT_SIZE(7);
      StaticJsonBuffer<capacity_data> jb_data;
      JsonObject& doc = jb_data.createObject();
      doc["type"].set("response");
      doc["BPM"].set(BPM);
      doc["IE_ratio"].set(IE_ratio);
      doc["maskPressure"].set(maskPressure);
      doc["flowRate"].set(volFlow);
      doc["TidVol"].set(TidVol);
      doc["totVolume"].set(totVolume);
      doc.printTo(SUART);
      doc.printTo(Serial);
      }
      }


// =====================
// Print to Screen
// =====================

void send_to_screen_graph() {
  String to_send_p_mask = ad + id_1 + "," + ch + "," + int(maskPressure);
  print_screen(to_send_p_mask);
  String to_send_volFlow = ad + id_2 + "," + ch + "," + int(volFlow);
  print_screen(to_send_volFlow);
  String to_send_totVolume = ad + id_3 + "," + ch + "," + int(totVolume);
  print_screen(to_send_totVolume);
}

void send_to_screen_values() {
  //dtostrf(BPM, 6, 2, buffer_1);
  String data = "page0.t_BPM.txt=\"" + String(BPM)  + "\"";writeString(data);
  //BPMText_Page1.setText(buffer);
  //dtostrf(IE_ratio, 6, 2, buffer_2);
   data = "page0.t_ie_ratio.txt=\"" + String(IE_ratio)  + "\"";writeString(data);
  //IERatioText_Page1.setText(buffer);
  //dtostrf(TidVol, 6, 2, buffer_3);
   data = "page0.t_tidvol.txt=\"" + String(TidVol)  + "\"";writeString(data);
  //TidVolText_Page1.setText(buffer_3);
  //t_mode.setText(set_mode.c_str());
}

void print_screen(String to_send) {
  Serial.print(to_send);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
