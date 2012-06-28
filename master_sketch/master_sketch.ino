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

void setup(){


}

void loop(){



}

void unlock(byte column, byte cell){
  getCommand(0);
  Message msg = Message(column, cell, flash_buffer);
  emptyBuffer();
}

void query(byte column, byte cell){
    getCommand(1);
    Message(column, cell, flash_buffer); 
    emptyBuffer();
}

void set_new_address(byte column, byte cell){
   getCommand(2);
    Message(column, cell, flash_buffer);
   emptyBuffer(); 
}

void transmit(Message msg){
   Wire.beginTransmission(msg.col);
   Wire.write(msg.serialize());  
 }

void getCommand(int i){
	strcpy_P(flash_buffer, (char*)pgm_read_word(&(string_table[i])));
}

void emptyBuffer(){
   memset(flash_buffer, 0, sizeof flash_buffer); 
}





