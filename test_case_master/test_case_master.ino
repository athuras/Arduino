#include <Wire.h>

byte frontEndInput[3];
boolean isInputComplete = false;
byte response[3];
boolean isResponseComplete = false;

void setup(){
  Serial.begin(9600);
  Wire.begin();
}

void loop(){
  serialEvent();
  if (isInputComplete){
      if (frontEndInput[0] == 'A'){ //set pin to high
          byte result = 0;
          //setup command to slave
          Wire.beginTransmission(99);
          Wire.write(frontEndInput, 3);
          result = Wire.endTransmission();
          
          requestCallBack();
          
      }
      if (frontEndInput[0] == 'B'){ //set pin to low
          byte result = 0;
          //setup command to slave
          Wire.beginTransmission(99);
          Wire.write(frontEndInput, 3);
          result = Wire.endTransmission();
          requestCallBack();
        
      }
      if (frontEndInput[0] == 'C'){ //get status of pin
         Wire.beginTransmission(99);
          Wire.write(frontEndInput, 3);
          Wire.endTransmission();
          requestCallBack();      
      }
      if (frontEndInput[0] == 'D'){ //lock
        
      }
           isInputComplete = false;

    }
    
    if (isResponseComplete){
        for (int i = 0; i < 3; i++){
           Serial.write(response[i]); 
        }
        isResponseComplete = false; 
    }
 
}

void serialEvent(){
   int cnt = 0;
   while (Serial.available()){
      if (cnt < 3){
        char inChar = (char)Serial.read();
        frontEndInput[cnt] = inChar;
        cnt++;
        if (inChar == '\n'){
           isInputComplete = true;   
        }
      } if (cnt == 3){
         Serial.read(); 
      }
   } 
}

void requestCallBack(){
  
            Wire.requestFrom(99, 3);
          int cnt = 0;
         while (Wire.available()){
            if (cnt < 3){
              char inChar = Wire.read();
              response[cnt] = inChar;
              cnt++;
            } if (cnt == 3){
               isResponseComplete = true;
               Wire.read(); 
            }
       } 
}
