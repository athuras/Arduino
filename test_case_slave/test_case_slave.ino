#include <Wire.h>

char input[3];
boolean isInputComplete = false;
String output;

void setup(){
   Wire.begin(99); 
   Wire.onReceive(receiveEvent);
   Wire.onRequest(requestEvent);
   output.reserve(24);
   pinMode(13, OUTPUT);
}

void loop(){
  if (isInputComplete){
     if (input[0] == 'A'){
        if (input[1] == '3'){
           digitalWrite(13, HIGH); 
        }
     } 
     if (input[0] == 'B'){
        if (input[1] == '3'){
           digitalWrite(13, LOW);
        } 
     }
  }
  isInputComplete = false;
  delay(100);
}

void receiveEvent(int howMany){
   int cnt = 0;
   while (Wire.available()){
      if (cnt < 3){
        char inChar = (char)Wire.read();
        input[cnt] = inChar;
        cnt++;
        if (inChar == '\n'){
           isInputComplete = true;   
        }
      } if (cnt == 3){
         Wire.read(); 
      }
   } 
}

void requestEvent(){
  if (input[0] == 'A'){
     Wire.write("AAA"); 
  } if (input[0] == 'B'){
    Wire.write("BBB");
  }
}
