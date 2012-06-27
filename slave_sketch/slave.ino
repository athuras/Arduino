// This is the slave microcontroller sketch

/*
Needs to know:
  List of Cells
  I2C Address
  Connection/Pin Schema for logic
  ADC calibration data
Needs to perform:
  Respond to Query Command
    ADC sensor data
  Respond to Unlock Command
*/

#include <Wire.h>

const int DEFAULT_ADDRESS = 1;

void setup(){
  Wire.begin(DEFAULT_ADDRESS);
  Wire.onRecieve(receiveEvent);
  Serial.begin(9600);
}

void loop(){

}

void receiveEvent(int value){

  // do somthing

}
