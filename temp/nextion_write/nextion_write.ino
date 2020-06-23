#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); /*Even though you can use the hardware serial port in this case I think it is better to
  leave the hardware serial open for debugging purposes*/

void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT); //This is our analog input pin

  Serial.begin(57600); //open the hardware serial port
  while (!Serial) { // wait for serial port to connect. Needed for native USB port only
    ;
  }

  Serial.println("Serial On"); //Print this messages when the serial port is connected
  mySerial.begin(9600); // set the data rate for the SoftwareSerial port
}

void loop() {
  String sendThis = ""; //Declare and initialise the string we will send

  delay(300); //Probably unneccessary, but I give the screen some time to respond
  sendThis = "n0.val="; //Build the part of the string that we know
  sendThis.concat(analogRead(0)); //Add the variable we want to send
  writeString(sendThis); /*Use a function to write the message character by character to the Nextion because
  mySerial.write(sendThis) gives you an error due to a datatype mismatch*/
}

//NOTE: A great big thanks to: RamjetX for writing this function. You can find his/her post here: http://forum.arduino.cc/index.php?topic=89143.0. Please go give him/her some Karma!
void writeString(String stringData) { // Used to serially push out a String with Serial.write()

  for (int i = 0; i < stringData.length(); i++)
  {
    mySerial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass  
  }

  mySerial.write(0xff); //We need to write the 3 ending bits to the Nextion as well
  mySerial.write(0xff); //it will tell the Nextion that this is the end of what we want to send.
  mySerial.write(0xff);

}// end writeString function
