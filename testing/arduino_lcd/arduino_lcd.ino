
#include <Servo.h>
#include<SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <Nextion.h>

// ==== Analog Pins =====
float potpinIE_ratio = 0;
int potpinTidVol = 1;
int potpinBPM = 5;
int pinguage_mask = 4;
int pinguage_expiration = 3;
int pinguage_diff = 2;

// ===== Other vars ======
float check_value;
float IE_ratio;
float TidVol;
float BPM;
float separation;
float sensorvalue;
//float pressure_mask;
//float pressure_expiration;
//float pressure_diff;

//====== Serial COnnection with NODEMCU =====
SoftwareSerial SUART(2, 3); //SRX=Dpin-2; STX-DPin-3

//====== LCD Variables ==============
int id_1 = 5;
int ch = 0;
int id_2 = 4;
int id_3 = 3;
String ad = "add ";
static char buffer[10] = {};
static char buffer_2[10] = {};
static char buffer_3[10] = {};
static char buffer_mode[10] = {};
String set_mode = "ACV";
NexText t_ie_ratio = NexText(0, 8, "t_ie_ratio");
NexText t_bpm = NexText(0, 9, "t_bpm");
NexText t_tidvol = NexText(0, 10, "t_tidvol");
NexText t_mode = NexText(0, 12, "t_mode");
NexButton b0 = NexButton(0, 2, "b0");
NexButton b1 = NexButton(0, 3, "b1");

float pressure_mask = 4.55;
float pressure_diff = 5.55;
float pressure_expiration = 6.55;
      
NexTouch *nex_listen_list[] = {&b0, &b1, NULL};


void b0PopCallback(void *ptr) {
  set_mode = "ACV";
  //set_mode.toCharArray(buffer_mode,5);
  t_mode.setText(set_mode.c_str());
}

//Button b1 component popcallback function
// When OFF button is released the LED turns OFF and the state text changes

void b1PopCallback(void *ptr) {
  set_mode = "SIMV";
  t_mode.setText(set_mode.c_str());
}


// ==== Sensor Offset =====
const float guageSensorOffset = 41;

const float pressureDiffSensorOffset = 41;

//===== Variables =====
Servo servoright;
int pos = 0;

//======= SD Card File ===========
File myFile;

String mode = "";

void setup() {
  Serial.begin(9600);
  SUART.begin(9600);
  servoright.attach(9);
  pinMode(10, OUTPUT);
  if (!SD.begin(4)) {
   Serial.println("initialization failed!");
   return;
  }
  Serial.println("SD Card Initialization done.");
  nexInit();
  b0.attachPop(b0PopCallback, &b0);
  b1.attachPop(b1PopCallback, &b1);

}

void loop() {
  // put your main code here, to run repeatedly:
    myFile = SD.open("example.txt", FILE_WRITE);
    if(myFile){
      Serial.println("The file is open");
      myFile.close();
    }
    else
    {
      Serial.println("Error opening this file");
      myFile.close();
      }
    // Run ventilator
    send_to_screen_values(BPM, IE_ratio, TidVol);
    if (set_mode == "ACV") {
    myFile = SD.open("example.txt", FILE_WRITE);
    acv_mode();
    myFile.close();
  }
  else if (set_mode == "SIMV") {
    myFile = SD.open("example.txt", FILE_WRITE);
    simv_mode();
    myFile.close();
  }
    
}

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


  // Fetch all potentiometer values
  IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);
  TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);
  BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);
  separation = (60 / BPM - (1 + IE_ratio));
  if (separation < 0)
  { IE_ratio = 60 / BPM - 1;
    separation = 60 / BPM - (1 + IE_ratio);
  }

  // Fetch pressure sensor values
  pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset) * 10.1972 / 92;
  pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset) * 10.1972 / 92;
  pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset) * 10.1972 / 92;
  // Initiate the cycle
  if (firstRun)
  {
    inspiration(TidVol, mode);
    delay(15);
    cycleEndTime = expiration(TidVol, IE_ratio, mode);
    firstRun = false;
  }

  // ========= Identify trigger and initiate the cycle =============
  if (millis() - cycleEndTime >= (uint32_t)separation * 1000 || pressure_mask < -1)
  {
    inspiration(TidVol, mode);
    delay(15);
    cycleEndTime = expiration(TidVol, IE_ratio, mode);
  }
  send_to_screen_values(BPM, IE_ratio, TidVol);
}

void inspiration(float TidVol, String mode)
{
  int pos;
  for (pos = 30; pos <= TidVol + 30; pos += 2) // goes from 0 degrees to 180 degrees
  {
    
    servoright.write(pos);
    delay(1000 / TidVol);

    // ============ Update pressure values =========
    //pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
    //pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
    //pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
      String data = mode + "," + String(pressure_mask) + "," + String(pressure_diff) + "," + String(pressure_expiration) + ";";
      myFile.println(data);
      send_to_screen_graph(pressure_mask, pressure_expiration, pressure_diff);
      //send_to_screen_values(BPM, IE_ratio, TidVol);
      //nexLoop(nex_listen_list);
  }
  transmit(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
}

// =====================
// Expiration Function
// =====================

uint32_t expiration(float TidVol, float IE_ratio, String mode)
{
  for (pos = TidVol + 30; pos >= 30; pos -= 2) // goes from 180 degrees to 0 degrees
  { 
    //nexLoop(nex_listen_list);
    servoright.write(pos);
    delay(1000 * IE_ratio / TidVol);
    // ============ Update pressure values =========
    //pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
    //pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
    //pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
      //Serial.println("Here inside inspiration");
      String data = mode + "," + String(pressure_mask) + "," + String(pressure_diff) + "," + String(pressure_expiration) + ";";
      myFile.println(data); 
      send_to_screen_graph(pressure_mask, pressure_expiration, pressure_diff);
      //send_to_screen_values(BPM, IE_ratio, TidVol);
  }
  transmit(BPM, IE_ratio, pressure_mask, pressure_expiration, TidVol, pressure_diff);
  return millis();
}

void simv_mode()
{
  uint32_t cycleEndTime;
  bool firstRun = true;
  mode = "SIMV";
  // Fetch all potentiometer values
  IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);
  TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);
  BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);
  separation = (60 / BPM - (1 + IE_ratio));
  if (separation < 0)
  { IE_ratio = 60 / BPM - 1;
    separation = 60 / BPM - (1 + IE_ratio);
  }
  // Fetch pressure sensor values
  pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset) * 10.1972 / 92;
  pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset) * 10.1972 / 92;
  pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset) * 10.1972 / 92;


  // Initiate the cycle
  if (firstRun)
  {
    pressure_mask = average_pressure_mask();
    cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio, mode);
    firstRun = false;
  }

  // ========= Identify trigger and initiate the cycle =============
  if (millis() - cycleEndTime >= (uint32_t)separation * 1000 || average_pressure_mask() < -1)
  {
    pressure_mask = average_pressure_mask();
    cycleEndTime = simv_logic(pressure_mask, TidVol, IE_ratio, mode);
  }
  send_to_screen_values(BPM, IE_ratio, TidVol);
}


// =======================
// SIMV Logic Function
// =======================

uint32_t simv_logic(float pressure_mask, float TidVol, float IE_ratio, String mode)
{
  uint32_t cycleEndTime;
  pressure_mask = int(floor(pressure_mask));
  switch (int(pressure_mask)) {
    case -6:
    case -5:
    case -4:
      TidVol = TidVol * 0.25;
      inspiration(TidVol, mode);
      delay(15);
      cycleEndTime = expiration(TidVol, IE_ratio, mode);
      break;
    case -3:
      TidVol = TidVol * 0.50;
      inspiration(TidVol, mode);
      delay(15);
      cycleEndTime = expiration(TidVol, IE_ratio, mode);
      break;
    case -2:
      TidVol = TidVol * 0.75;
      inspiration(TidVol, mode);
      delay(15);
      cycleEndTime = expiration(TidVol, IE_ratio, mode);
      break;
    case -1:
      TidVol = TidVol * 0.75;
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
// Average Pressure Function
// =======================

float average_pressure_mask()
{
  pressure_mask = 0;
  for (int i = 0; i < 5; i++)
  {
    pressure_mask = pressure_mask + (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset) * 10.1972 / 92;
    delay(3);
  }
  return (pressure_mask/5);
}


// =====================
// Print to Screen
// =====================

void send_to_screen_graph(float pressure_mask, float pressure_expiration, float pressure_diff) {
  float p_mask = map(pressure_mask, 5.00, 20.00, 0, 255);
  float p_expiration = map(pressure_mask, 5.00, 20.00, 0, 255);
  float p_diff = map(pressure_mask, 5.00, 20.00, 0, 255);
  String to_send_p_mask = ad + id_1 + "," + ch + "," + int(p_mask);
  print_screen(to_send_p_mask);
  String to_send_p_diff = ad + id_2 + "," + ch + "," + int(p_diff);
  print_screen(to_send_p_diff);
  String to_send_p_exp = ad + id_3 + "," + ch + "," + int(p_expiration);
  print_screen(to_send_p_exp);
}

void send_to_screen_values(float BPM, float IE_ratio, float TidVol) {
  dtostrf(BPM, 6, 2, buffer);
  t_bpm.setText(buffer);
  dtostrf(IE_ratio, 6, 2, buffer_2);
  t_ie_ratio.setText(buffer_2);
  dtostrf(TidVol, 6, 2, buffer_3);
  t_tidvol.setText(buffer_3);
  t_mode.setText(set_mode.c_str());
}

void print_screen(String to_send) {
  Serial.print(to_send);
  Serial.print("\xFF\xFF\xFF");
}


// =====================
// Transmit to DB
// =====================

void transmit(float BPM, float IE_ratio, float pressure_mask, float pressure_expiration, float TidVol, float pressure_diff){
      Serial.println("Inside the transmit function");
      String message = "";
      boolean messageReady = false;
      while(SUART.available()) {
        message = SUART.readString();
        Serial.println(message);
        messageReady = true;
        }
      if(messageReady) {
      // The only messages we'll parse will be formatted in JSON
      
      const int capacity_data = JSON_OBJECT_SIZE(7);
      StaticJsonBuffer<capacity_data> jb_data;
      JsonObject& doc = jb_data.createObject();
      doc["type"].set("response");
      doc["BPM"].set(BPM);
      doc["IE_ratio"].set(IE_ratio);
      doc["pressure_mask"].set(pressure_mask);
      doc["pressure_expiration"].set(pressure_expiration);
      doc["TidVol"].set(TidVol);
      doc["pressure_diff"].set(pressure_diff);
      doc.printTo(SUART);
      doc.printTo(Serial);
      }
      }

      
