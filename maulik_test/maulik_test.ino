#include <Servo.h>

Servo servoright;

void setup() {
  // put your setup code here, to run once:
servoright.attach(9);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  // put your main code here, to run repeatedly:
  int pos;
  for(pos = 0; pos <= 90; pos += 1) // goes from 0 degrees to 180 degrees
    {                                  // in steps of 1 degree
      servoright.write(pos);
      delay(10);                       
  
      // ============ Update pressure valu
      //Serial.println(pressure_diff);
      //Serial.println(pressure_expiration);
    }
  delay(3000);
   for(pos = 90; pos >= 0; pos -= 1) // goes from 0 degrees to 180 degrees
    {                                  // in steps of 1 degree
      servoright.write(pos);
      delay(10);                       
  
      // ============ Update pressure valu
      //Serial.println(pressure_diff);
      //Serial.println(pressure_expiration);
    }
    delay(3000);
}
