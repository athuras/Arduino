#include <Wire.h>

char input[3];
String output;

void setup(){
   Wire.begin(99); 
   Wire.onReceive(receiveEvent);
   Wire.onRequest(requestEvent);
   output.reserve(24);
}

void loop(){
  delay(100);
}

void receiveEvent(int howMany){
    byte cnt = 0;
    while (Wire.available() && cnt < 3){
       char c = Wire.read();
       input[cnt] = c;
       cnt++;
    }
    if (input[0] == 'A'){
       byte pinNum = input[1];
       pinMode(pinNum, OUTPUT);
       digitalWrite(pinNum, HIGH); 
       output = "Pin is high";
    }
}

void requestEvent(){
   if (input[0] == 'A'){
     Wire.write(output);
   }  
}
