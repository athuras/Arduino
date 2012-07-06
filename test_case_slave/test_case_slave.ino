#include <Wire.h>

char input[3];
boolean isInputComplete = false;
String output;
int readValue;

boolean lockCycleOn = false;
unsigned long lockCycleStartTime;
byte lockedAddress = 0;

void setup(){
  Wire.begin(99); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  output.reserve(24);
  pinMode(13, OUTPUT);
}

void loop(){
  if (isInputComplete && !lockCycleOn){
    byte c = parseAddress((byte*)input);
    if (input[0] == 'A'){
      pinMode(c, OUTPUT);
      digitalWrite(c, HIGH);
    } 
    if (input[0] == 'B'){
      pinMode(c, OUTPUT);
      digitalWrite(c, LOW);
    }
    if (input[0] == 'C'){
      pinMode(c, INPUT);
      readValue = digitalRead(c);
    }
    if (input[0] == 'D'){
      pinMode(c, OUTPUT);
      digitalWrite(c, HIGH);
      lockCycleOn = true;  
      lockCycleStartTime = millis();   
      lockedAddress = c;
    }
  }
  checkLockCycle();
  isInputComplete = false;
  delay(100);
}

//called every loop to check the line lock caused by unlocking a locker
//dummied out to lock for a time interval
//in read implementation, would check for the limit switch of the opened lock
void checkLockCycle(){
   if (lockCycleOn && millis() - lockCycleStartTime >= 5000){
     digitalWrite(lockedAddress, LOW);
     lockCycleOn = false;
     lockCycleStartTime = 0;
  }
}

byte parseAddress(byte* buffer){
  /*
  //for two digit addresses, enable later
  byte temp = 0;
  if (buffer[1] >=48 && buffer[1] <= 57){
    temp += (buffer[1] - 48)*10;
  }
  if (buffer[2] >=48 && buffer[2] <= 57){
    temp += (buffer[2]-48);
  }
  return temp;
  */
  if (buffer[1] >=48 && buffer[1] <= 57){
     return (buffer[1] - 48); 
  }
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
    } 
    if (cnt == 3){
      Wire.read(); 
    }
  } 
}

void requestEvent(){
  if (lockCycleOn){
    Wire.write("XXX");
  } else {
    if (input[0] == 'A'){
      Wire.write("AAA"); 
    } 
    if (input[0] == 'B'){
      Wire.write("BBB");
    } 
    if (input[0] == 'C'){
      if (readValue == 0){
        Wire.write("C00");
      } 
      else if (readValue == 1){
        Wire.write("C11");
      } 
    }
    if (input[0] == 'D'){
       Wire.write("DDD"); 
    }
  }
  

}

void writeToWire(String str){
  for (byte i = 0; i < str.length(); i++){
     Wire.write(str.charAt(i)); 
  }
}

void writeToWire(byte* buffer, byte length){
  
}

