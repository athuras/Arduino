#include <avr/pgmspace.h>


//set up commands into flash memory
prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51} ; 

PROGMEN const char* string_table[] = {
	unlockcode,
	querycode,
	new_addresscode
};

//global variable
//temporary buffer to hold commands extracted from flash memory
char flash_buffer[8]; 

void setup(){


}

void loop(){



}

void unlock(byte column, byte cell){
	getCommand(0);
	Message(column, cell, buffer);

}

char* getCommand(int i){
	strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i])));
}



