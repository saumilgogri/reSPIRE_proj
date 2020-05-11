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

// ==== Analog Pins =====

int inPin = 2;

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

// ======= Mode changing vars =========

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin
long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  attachInterrupt(0, pin_ISR, CHANGE);
}
 
void loop()
 
{
  // ** Main update call for the guino
  //guino_update();
  // ========== Read all analog input & Detecting Mode Settings =============
  pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
  pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
  pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
  reading = digitalRead(inPin)
  if (reading == HIGH && previous == LOW && millis() - time > debounce) {
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
  else if((reading == LOW && previous == HIGH && millis() - time > debounce) {
      valTidVol = analogRead(potpinTidVol);            // reads the value of the potentiometer (value between 0 and 1023)
      valTidVol = map(valTidVol, 0, 1023, 40.00, 90.00);     // scale it to use it with the servo (value between 0 and 180)
      acv_mode(valTidVol,valBPM,valIE_ratio)
    }
previous = reading;
}

void acv_mode(acv_volume,acv_bpm,acv_ieratio){
  per_breath_time = 1/acv_bpm*60;
  per_inspiration_time = 1/(1+1/acv_ieratio)*per_breath_time;
  per_expiration_time = per_breath_time - per_inspiration_time;
  for(int i=0;i<5;i++){
  human_effort = analogRead(pinguage_mask);
  delay(5);
  }
  human_effort = analogRead(pinguage_mask);
  if( human_effort < 0){
    if( human_effort < -100 && human_effort > -200){
    acv_volume = 0.8*acv_volume;
    per_inspiration_time = 0.8*per_inspiration_time;
    delay(per_inspiration_time);
    acv_ie_ratio = per_inspiration_time/per_expiration_time; 
    }
    else if (human_effort < -200 && human_effort > -300){
    acv_volume = 0.6*acv_volume;
    per_inspiration_time = 0.6*per_inspiration_time;
    delay(per_inspiration_time);
    acv_ie_ratio = per_inspiration_time/per_expiration_time;
    }
    else if (human_effort < -300 && human_effort > -400){
    acv_volume = 0.4*acv_volume;
    per_inspiration_time = 0.4*per_inspiration_time;
    delay(per_inspiration_time);
    acv_ie_ratio = per_inspiration_time/per_expiration_time;
    }
    else if (human_effort < -400){
    acv_volume = 0.2*acv_volume;
    per_inspiration_time = 0.2*per_inspiration_time;
    delay(per_inspiration_time);
    acv_ie_ratio = per_inspiration_time/per_expiration_time;
    }
  }
  // ====== Copmute respiration separation time based on pot inputs ==========
  separation = 60/acv_bpm - (1+acv_ie_ratio);
  if (separation < 0)
  {
    acv_ie_ratio = 60/acv_bpm - 1;
    separation = 60/acv_bpm - (1+acv_ie_ratio);
  }

  // ========= Servo a-Clockwise motion =============
  for(pos = 30; pos <= acv_vol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servoright.write(pos);
    delay(1000/acv_vol);                       

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
  for(pos = acv_vol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servoright.write(pos);
    delay(1000*acv_ie_ratio/acv_vol);                       
    // ============ Update pressure values =========
    pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
    pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92; 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    
    Serial.println(pressure_mask);
    Serial.println(pressure_diff);
    Serial.println(pressure_expiration);
  }  
  delay(separation*1000);
  end_time = millis();
  
}
}

