#include <Wire.h>

char frontEndInput[3];
boolean isInputComplete = false;
char response[24];
boolean isResponseComplete = false;

void setup(){
  Serial.begin(9600);
  Wire.begin();
}

void loop(){
  if (isInputComplete){
      if (frontEndInput[0] == 'A'){ //set pin to high
          byte result = 0;
          //setup command to slave
          Wire.beginTransmission(99);
          Wire.write(frontEndInput, 3);
          result = Wire.endTransmission();
          if (result != 0){
             //we are in error 
          }
          //end command
          
          //request acknowledgement
          Wire.requestFrom(99, 24);
          
          byte cnt = 0;
          isResponseComplete = false;
          while (Wire.available()){
              char c = (char)Wire.read();
              response(cnt) = c;
              cnt++;
              if (c == '\n'){
                  isResponseComplete = true;
              }
          }
          while(cnt < 24){
             response(cnt) = 0;
             cnt ++; 
          }
      }
      if (frontEndInput[0] == 'B'){ //set pin to low
        
      }
      if (frontEndInput[0] == 'C'){ //get status of pin
        
      }
      if (frontEndInput[0] == 'D'){ //lock
        
      }
  }
}

void serialEvent(int var){
   byte cnt = 0;
   while (Serial.available() && cnt < 3){
      char inChar = (char)Serial.read();
      frontEndInput[cnt] = inChar;
      cnt++;
      if (inChar == '\n'){
         isInputComplete = true; 
      }
   } 
}
