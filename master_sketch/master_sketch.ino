
#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>


//set up commands into flash memory
prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51} ; 
prog_char limitswitchcode[] PROGMEM = {52, 52, 52, 52, 52, 52, 52, 52};

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{unlockcode, querycode, new_addresscode, limitswitchcode};

bool isInputComplete = false;
bool isSlaveResponseComplete = false;
byte toSlaveBuffer[10];
byte toFrontBuffer[10];
byte fromSlaveBuffer[10];
byte fromFrontBuffer[10];
byte slaveResponseLength = 10;

bool inLockCycle = false;
int lockDelay = 100;
byte lockedColumn = 0;
byte lockedCell = 0;

void setup(){
  Serial.begin(9600);
  Wire.begin();
}

void loop(){
	serialEvent();
	if (inLockCycle){
		Message msg = Message(lockedColumn, lockedCell, string_table[3]);
		if (writeToSlave(msg) == 0){
			requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
		}
		/*
		Pseudo-code
		If lock is still open:
			remain in lockcycle
		If lock is closed:
			Query the analog sensor
			Send result to front-end
			inLockCycle = false; //to leave
		*/
	}
	if (isInputComplete && !inlockCycle){
		byte col = charNumToByteNum((char)fromFrontBuffer[0]);
		byte cell = charNumToByteNum((char)fromFrontBuffer[1]);
		byte command = parseFrontEndCommand(fromFrontBuffer); //wrap this into an enum
		if (command == 0){ //unlock
			Message msg = Message(col, cell, string_table[0]);  
			if (writeToSlave(msg) == 0){
				requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
				inLockCycle = true;
				lockedColumn = col;
				lockedCell = cell;
			} else {
			  
			}
		}
		if (command == 1){ //query analog sensor
			Message msg = Message(col, cell, string_table[1]); 
			if (writeToSlave(msg) == 0){
				requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
			} else {
		  
			}
		}
		if (command == 2){ //query limit switch
			Message msg = Message(col, cell, string_table[3]); 
			if (writeToSlave(msg) == 0){
				requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
			
			} else {
		  
			}      
		}
		if (command == 3){ //query all limit switches
			Message msg = Message(col, 0, string_table[3]); 
			if (writeToSlave(msg) == 0){
				requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
			} else {
		  
			}      
		}      
   }
   delay(100);
}

byte writeToSlave(Message msg){
  int length = 10; //this is dummied
  char writeBuffer[length];
  msg.serialize(writeBuffer, length);
  	Wire.beginTransmission(msg.col);
	Wire.write((byte*)writeBuffer, length);
	return Wire.endTransmission();
}

void requestCallBack(byte column, byte* buffer, byte length){
  Wire.requestFrom(column, length);
  int cnt = 0;
  while (Wire.available()){
    if (cnt < length){
      char inChar = Wire.read();
      buffer[cnt] = inChar;
      cnt++;
    } 
    if (cnt == length){
      isSlaveResponseComplete = true;
      Wire.read(); 
    }
  } 
}

byte charNumToByteNum(char c){
  if (c >=48 && c <= 57){
     return (c - 48); 
  } 
}

void serialEvent(){
  readByteArrayFromSerial(fromFrontBuffer, 10);
}

//helper function to read null terminated character arrays 
//null terminator is included in the length
void readCharArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, true);
}

//helper function to read byte array of length length
void readByteArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, false);
}

//reads request length from serial into specified 8bit array
//after requested length is read, continues to read off the line until Serial is empty
void readArrayFromSerial(byte* buffer, byte length, boolean isNullTerminated){
	int cnt = 0;
	while (Serial.available()){
		if (cnt < length){
			char temp = (char)Serial.read();
			buffer[cnt] = temp;
			cnt++;
			if (isNullTerminated && (char)temp == '\n'){
				isInputComplete = true;
			}
		}
		if (cnt == length){
			isInputComplete = true;
			Serial.read();
		}
	}
}	

void writeToFront(byte* message, byte length){
	for (byte i = 0; i < length; i++){
		Serial.write(message[i]);
	}
}

void writeToFront(String str){
	for (byte i = 0; i < str.length(); i++){
		Serial.write(str.charAt(i));
	}
}

byte parseFrontEndCommand(byte* command){
	if ((char)command[2] == 'A'){
		return 0;
	}
	if ((char)command[2] == 'B'){
		return 1;
	}
	if ((char)command[2] == 'C'){
		return 2;
	}
	if ((char)command[2] == 'D'){
		return 3;
	}
}