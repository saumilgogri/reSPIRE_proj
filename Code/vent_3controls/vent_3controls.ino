#include <Guino.h>

#include <Servo.h>
 
Servo servoright;
int pos = 0;    

// ==== Analog Pins =====
float potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 5;
int pinguage_mask = 4;
int pinguage_expiration = 3;
int pinguage_diff = 2;

// ==== Sensor Offset =====
const float guageSensorOffset = 41;
const float pressureDiffSensorOffset = 41;

// ===== Other vars ======
float valIE_ratio;  
float valTidVol;  
float valBPM;
float separation;  
float sensorvalue;
float pressure_mask;
float pressure_expiration;
float pressure_diff;

int display_IE_ratio;
int display_valBPM;
int display_valTidVol;
int display_sensorvalue_cmh2o;
int display_sensorvalue_gp_cmh2o;



void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}
 
void loop()
 
{
  // ** Main update call for the guino
  //guino_update();
  // ========== Read all analog input =============
  pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
  pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
  pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
  valIE_ratio = analogRead(potpinIE_ratio);            // reads the value of the potentiometer (value between 0 and 1023)
  valIE_ratio = map(valIE_ratio, 0, 1023, 1.00, 4.00);     // scale it to use it with the servo (value between 0 and 180)
  valTidVol = analogRead(potpinTidVol);            // reads the value of the potentiometer (value between 0 and 1023)
  valTidVol = map(valTidVol, 0, 1023, 40.00, 90.00);     // scale it to use it with the servo (value between 0 and 180)
  valBPM = analogRead(potpinBPM);            // reads the value of the potentiometer (value between 0 and 1023)
  valBPM = map(valBPM, 0, 1023, 8.00, 30.00);     // scale it to use it with the servo (value between 0 and 180)
 
  // ====== Copmute respiration separation time based on pot inputs ==========
  separation = 60/valBPM - (1+valIE_ratio);
  if (separation < 0)
  {
    valIE_ratio = 60/valBPM - 1;
    separation = 60/valBPM - (1+valIE_ratio);
  }

  // ========= Servo a-Clockwise motion =============
  for(pos = 30; pos <= valTidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servoright.write(pos);
    delay(1000/valTidVol);                       

    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
    Serial.println(pressure_mask);
    Serial.println(pressure_diff);
    Serial.println(pressure_expiration);
  }
  delay(15);
  
  // ========= Servo Clockwise motion ============
  for(pos = valTidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servoright.write(pos);
    delay(1000*valIE_ratio/valTidVol);                       
    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
    Serial.println(pressure_mask);
    Serial.println(pressure_diff);
    Serial.println(pressure_expiration);
  }  
  delay(separation*1000);
}
