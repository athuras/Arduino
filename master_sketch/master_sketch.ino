#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>

//set up commands into flash memory
prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51} ; 

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{   
  unlockcode, querycode, new_addresscode};

//global variable
//temporary buffer to hold commands extracted from flash memory
char flash_buffer[8]; 

//global variables to store input from over the serial line
String frontend_input = "";
boolean stringComplete = false;

const byte FACTORY_ADDRESS = 99;
const byte RESPONSE_LENGTH = 24;

void setup(){
  Wire.begin();
  Serial.begin(9600);
  frontend_input.reserve(200);
}

void loop(){
	//on receiving instruction from front-end
	if (stringComplete) {
		
	}
	
	//probe factory address location
	if (probe_default()){
		//begin add new address routine
	} 
	delay(100);
}

/*
	Purpose: 	Determines if a device is at default factory location
	Output: 	True if there is a device residing at the default factory location
				False if no device at default factory location, or error in transmission
*/
boolean probe_default(){
   byte result = genericCommand(FACTORY_ADDRESS, 0, 1);
   if (result == 0){ 	//successful send and receive
		return true;
   }
   return false;
}

/*
	Purpose:	sends unlock command 
*/
void unlock(byte column, byte cell){
	//should make this function lock up the line with status request
   generic_command(column, cell, 0);
}

void query(byte column, byte cell){
   generic_command(column, cell, 1); //primes the slave to send status information on next request
   Wire.requestFrom(column, RESPONSE_LENGTH);
   while(Wire.availible()){
		//read the response
   }
}

void set_new_address(byte column, byte cell){
   generic_command(column, cell, 2);
   Wire.requestFrom(column, RESPONSE_LENGTH);
   while(Wire.availible()){
		//read the response
   }
}

/*
	Purpose: 	helper function to transmit messages to slaves
	Output: 	Returns result of the attempted transmission
*/
byte generic_command(byte column, byte cell, int command){
  getCommand(command);
  Message msg = Message(column, cell, flash_buffer);
  byte result = transmit(msg);
  emptyBuffer();
	return result;
}

/*
Transmits command to target location
Input: composed message with column, cell, and command
Output: 	0:success
			1:data too long to fit in transmit buffer
			2:received NACK on transmit of address
			3:received NACK on transmit of data
			4:other error
*/
byte transmit(Message msg){
   Wire.beginTransmission(msg.col);
   Wire.write(msg.serialize());
   return Wire.endTransmission();
 }

void getCommand(int i){
	strcpy_P(flash_buffer, (char*)pgm_read_word(&(string_table[i])));
}

void emptyBuffer(){
   memset(flash_buffer, 0, sizeof flash_buffer); 
}

/*
	Called between every loop called
	Tries to reach off serial line
*/
void serialEvent(){
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    frontend_input += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 

}





