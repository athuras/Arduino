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
		if(writeToSlave(99, frontEndInput, 3) == 0){
			requestCallBack(99, response, 3);
		} else {

		}
    }
    if (frontEndInput[0] == 'B'){ //set pin to low
		if(writeToSlave(99, frontEndInput, 3) == 0){
			requestCallBack(99, response, 3);
		} else {
			//in error
		}
    }
    if (frontEndInput[0] == 'C'){ //get status of pin
		if(writeToSlave(99, frontEndInput, 3) == 0){
			requestCallBack(99, response, 3);
		} else {
			//in error
		} 
    }
    if (frontEndInput[0] == 'D'){ //lock
         if (writeToSlave(99, frontEndInput, 3) == 0){
            requestCallBack(99, response, 3); 
         } else {
           
         }

    }
    isInputComplete = false;
  }

  if (isResponseComplete){
	writeToSerial(response, 3);
    isResponseComplete = false; 
  }

}

void serialEvent(){
  int cnt = 0;
  //readByteArrayFromSerial(frontEndInput, 3);
  while (Serial.available()){
    if (cnt < 3){
      char inChar = (char)Serial.read();
      frontEndInput[cnt] = inChar;
      cnt++;
      if (inChar == '\n'){
        isInputComplete = true;   
      }
    } 
    if (cnt == 3){
      Serial.read(); 
    }
  } 
}

void readCharArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, true);
}

void readByteArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, false);
}

void readArrayFromSerial(byte* buffer, byte length, boolean isNullTerminated){
	int cnt = 0;
	while (Serial.available()){
		if (cnt < length){
			char temp = (char)Serial.read();
			frontEndInput[cnt] = temp;
			cnt++;
			if (isNullTerminated && (char)temp == '\n'){
				isInputComplete = true;
			}
		}
		if (cnt == length){
			Serial.read();
		}
	}
}	

void requestCallBack(byte address, byte* buffer, byte length){
  Wire.requestFrom(address, length);
  int cnt = 0;
  while (Wire.available()){
    if (cnt < length){
      char inChar = Wire.read();
      buffer[cnt] = inChar;
      cnt++;
    } 
    if (cnt == length){
      isResponseComplete = true;
      Wire.read(); 
    }
  } 
}

byte writeToSlave(byte address, byte* message, byte length){
	Wire.beginTransmission(address);
	Wire.write(message, length);
	return Wire.endTransmission();
}

void writeToSerial(byte* message, byte length){
	for (byte i = 0; i < length; i++){
		Serial.write(message[i]);
	}
}

void writeToSerial(String str){
	for (byte i = 0; i < str.length(); i++){
		Serial.write(str.charAt(i));
	}
}
