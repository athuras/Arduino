
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
byte buffer[10];

void setup(){
  Serial.begin(9600);
  Wire.begin();
}

void loop(){
   serialEvent();
   if (isInputComplete){
      byte col = charNumToByteNum((char)buffer[0]);
      byte address = charNumToByteNum((char)buffer[1]);
       
   }
}

byte charNumToByteNum(char c){
  if (c >=48 && c <= 57){
     return (c - 48); 
  } 
}

void serialEvent(){
  readByteArrayFromSerial(buffer, 10);
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


