#include <Guino.h>

#include <Servo.h>
 
Servo servoright;
int pos = 0;    

# ==== Analog Pins =====
float potpinIE_ratio = 0;  
int potpinTidVol = 1;  
int potpinBPM = 5;
int pinguage_mask = 4;
const float SensorOffset = 41;
const float gp_sensor_offset = 41;
float valIE_ratio;  
float valTidVol;  
float valBPM;
float separation;  
  
int display_IE_ratio;
int display_valBPM;
int display_valTidVol;
int display_sensorvalue_cmh2o;
int display_sensorvalue_gp_cmh2o;
float sensorvalue;
float sensorvalue_cmh2o;
float sensorvalue_gp;
float sensorvalue_gp_cmh2o;



void setup()
{
  servoright.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
}
 
void loop()
 
{
  // ** Main update call for the guino
  //guino_update();
  sensorvalue_cmh2o = map(analogRead(potpincmH20), 0, 1023, 0, 1023);    
  //sensorvalue_cmh2o = (analogRead(A4)-SensorOffset)*10.1972/92 ;
  sensorvalue_gp_cmh2o = (analogRead(A5)-gp_sensor_offset)*10.1972/92;
  valIE_ratio = analogRead(potpinIE_ratio);            // reads the value of the potentiometer (value between 0 and 1023)
  valIE_ratio = map(valIE_ratio, 0, 1023, 1.00, 4.00);     // scale it to use it with the servo (value between 0 and 180)
  valTidVol = analogRead(potpinTidVol);            // reads the value of the potentiometer (value between 0 and 1023)
  valTidVol = map(valTidVol, 0, 1023, 40.00, 90.00);     // scale it to use it with the servo (value between 0 and 180)
  valBPM = analogRead(potpinBPM);            // reads the value of the potentiometer (value between 0 and 1023)
  valBPM = map(valBPM, 0, 1023, 8.00, 30.00);     // scale it to use it with the servo (value between 0 and 180)
 
 
  //gUpdateValue(&display_sensorvalue_cmh2o);
  separation = 60/valBPM - (1+valIE_ratio);
  if (separation < 0)
  {
    valIE_ratio = 60/valBPM - 1;
    separation = 60/valBPM - (1+valIE_ratio);
  }
 
  for(pos = 30; pos <= valTidVol+30; pos += 1) // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    servoright.write(pos);
    delay(1000/valTidVol);                       // waits 15ms for the servo to reach the position
    sensorvalue_cmh2o = map(analogRead(potpincmH20), 0, 1023,  0, 1023);    

    //sensorvalue_cmh2o = (analogRead(A4)-SensorOffset)*10.1972/92;
    sensorvalue_gp_cmh2o = (analogRead(A5)-gp_sensor_offset)*10.1972/92;
    //Serial.println("This is the Diffrential Pressure Value");
    Serial.println(sensorvalue_cmh2o);
    //Serial.println("This is the Gauge Pressure Value");
    //Serial.println(sensorvalue_gp_cmh2o);
  }
  //gUpdateValue(&display_sensorvalue_cmh2o);
  delay(15);
  for(pos = valTidVol+30; pos>=30; pos-=1)     // goes from 180 degrees to 0 degrees
  {                                
    servoright.write(pos);
    delay(1000*valIE_ratio/valTidVol);                       // waits 15ms for the servo to reach the position
    sensorvalue_cmh2o = map(analogRead(potpincmH20), 0, 1023,  0, 1023 );    
   
    //sensorvalue_cmh2o = (analogRead(A4)-SensorOffset)*10.1972/92;
    sensorvalue_gp_cmh2o = (analogRead(A5)-gp_sensor_offset)*10.1972/92;


    //Serial.println("This is the Diffrential Pressure Value");
    Serial.println(sensorvalue_cmh2o);
    //Serial.println("This is the Gauge Pressure Value");
    //Serial.println(sensorvalue_gp_cmh2o);
  }  
  delay(separation*1000);
  //gUpdateValue(&display_sensorvalue_cmh2o);

  //Serial.println("This is the valIE_ratio");
  //Serial.println(valIE_ratio);
  //Serial.println("This is the valBPM");
  //Serial.println(valBPM);
  //Serial.println("This is the valTidVol");
  //Serial.println(valTidVol);
  //Serial.println("This is the Diffrential Pressure Value");
  //Serial.println(sensorvalue_cmh2o);
  //Serial.println("This is the Gauge Pressure Value");
  //Serial.println(sensorvalue_gp_cmh2o);
 
  //display_IE_ratio = round(valIE_ratio);
  //display_valBPM = round(valBPM);
  //display_valTidVol = round(valTidVol);
  //display_sensorvalue_cmh2o = round(sensorvalue_cmh2o);
  //display_sensorvalue_gp_cmh2o = round(sensorvalue_gp_cmh2o);
 
 
  // Send the value to the gui.
  //gUpdateValue(&valTidVol);
  //gUpdateValue(&valBPM);
  //gUpdateValue(&display_IE_ratio);
  //gUpdateValue(&display_sensorvalue_cmh2o);
  //gUpdateValue(&display_sensorvalue_gp_cmh2o);

}
