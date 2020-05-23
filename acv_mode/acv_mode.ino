#include <Guino.h>
#include <Servo.h>
Servo servoright;
int pos = 0;    

// ==== Analog Pins =====
float potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 2;
int pinguage_mask = 5;
int pinguage_expiration = 4;
int pinguage_diff = 3;

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
uint32_t endTime;


// ======= Mode changing vars =========

int state = HIGH;      // the current state of the output pin
int mode;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin
long t_time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers


void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
//  attachInterrupt(0, pin_ISR, CHANGE);
}
 
void loop()
 
{
  // ========== Read all analog input & Detecting Mode Settings =============
  pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
  pressure_diff = (map(analogRead(pinguage_diff), 0, 1023, 0, 1023) - pressureDiffSensorOffset)*10.1972/92;
  pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
  
  mode = digitalRead(inPin);

  IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);     // scale it to use it with the servo (value between 0 and 180)
  TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);     // scale it to use it with the servo (value between 0 and 180)
  BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     // scale it to use it with the servo (value between 0 and 180)

  // ====== Copmute respiration separation time based on pot inputs ==========
  separation = 60/BPM - (1+IE_ratio);
  if (separation < 0)
  {
    IE_ratio = 60/BPM - 1;
    separation = 60/BPM - (1+IE_ratio);
  }

/*   if (mode == HIGH && previous == LOW && millis() - t_time > debounce) 
  {

      // ========= Servo a-Clockwise motion =============
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
      delay(15);
      
      // ========= Servo Clockwise motion ============
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
      delay(separation*1000);
  }
  else 
  {  */
    endTime = millis();  
    while(digitalRead(inPin) == HIGH)
      {
        pressure_diff = analogRead(pinguage_diff);   
        
        acv_mode();  
      }
      
//  }
previous = mode;

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

  while(digitalRead(inPin) == HIGH)
  {
      // Fetch all potentiometer values
      IE_ratio = map(analogRead(potpinIE_ratio), 0, 1023, 1.00, 4.00);    
      TidVol = map(analogRead(potpinTidVol), 0, 1023, 40.00, 90.00);     
      BPM = map(analogRead(potpinBPM), 0, 1023, 8.00, 30.00);     
      separation = (60/BPM - (1+IE_ratio));
      if (separation < 0)
      { 
        IE_ratio = 60/BPM - 1;
        separation = 60/BPM - (1+IE_ratio);
      }
      // Fetch pressure sensor values
      pressure_mask = (map(analogRead(pinguage_mask), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;    
      pressure_diff = analogRead(pinguage_diff);       
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


  }
  
  /* per_breath_time = 1/BPM*60;
  per_inspiration_time = 1/(1+1/IE_ratio)*per_breath_time;
  per_expiration_time = per_breath_time - per_inspiration_time;
  for(int i=0;i<5;i++){
  human_effort = analogRead(pinguage_mask);
  check_value = (map(analogRead(human_effort), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92;
  delay(5);
  }
  //human_effort = analogRead(pinguage_mask);
  if(check_value < -1){
    if( check_value < -0.5 && check_value > -1){
    TidVol = 0.8*TidVol;
    per_inspiration_time = 0.8*per_inspiration_time;
    delay(per_inspiration_time);
    IE_ratio = per_inspiration_time/per_expiration_time; 
    }
    else if (check_value < -1 && check_value  > -2){
    TidVol = 0.6*TidVol;
    per_inspiration_time = 0.6*per_inspiration_time;
    delay(per_inspiration_time);
    IE_ratio = per_inspiration_time/per_expiration_time;
    }
    else if (check_value  < -2 && check_value > -3){
    TidVol = 0.4*TidVol;
    per_inspiration_time = 0.4*per_inspiration_time;
    delay(per_inspiration_time);
    IE_ratio = per_inspiration_time/per_expiration_time;
    }
    else if (check_value < -4){
    TidVol = 0.2*TidVol;
    per_inspiration_time = 0.2*per_inspiration_time;
    delay(per_inspiration_time);
    IE_ratio = per_inspiration_time/per_expiration_time;
    }
  }
 */
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
    pressure_diff = analogRead(pinguage_diff); 
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    if (millis() - endTime > (uint32_t)100)
        {
          Serial.println(pressure_mask);
          endTime = millis();
        }
    //Serial.println(pressure_mask);
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
    pressure_diff = analogRead(pinguage_diff);   
    pressure_expiration = (map(analogRead(pinguage_expiration), 0, 1023, 0, 1023) - guageSensorOffset)*10.1972/92; 
    if (millis() - endTime > (uint32_t)100)
        {
          Serial.println(pressure_mask);
          endTime = millis();
        }
    //Serial.println(pressure_mask);
    //Serial.println(pressure_diff);
    //Serial.println(pressure_expiration);
  }  
  return millis();
}



// =============================
// Copmute volume and flow rates
// =============================
void computeFlows(float area_1, float area2)
{
  float pressureDiff;
  float pressure_cmH2o;
  float pressure_pa;
  float rho = 1.225;
  float massFlow;
  float volFlow;
  
  pressure_cmH2o = (analogRead(pinguage_diff) - pressureDiffSensorOffset)*10.1972/92;
  pressure_pa = pressure_cmH2o*98.0665;
//  massFlow=1000*sqrt((abs(pressure_pa)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2))))); 
  volFlow = massFlow/rho; 
  
} 
