#include "Message.h"
#include "string.h"

const int COMMAND_LENGTH = 8;
const int OUTPUT_LENGTH = 10;

Message::Message(){
	col = 0;
	cell = 0;
	memset(this->command, 0, COMMAND_LENGTH);
}

Message::Message(char col, char cell, char* command)
{
	this-> col = col;
	this-> cell = cell;
	memcpy(this->command, command, COMMAND_LENGTH);
}

void Message::serialize(char* output_array, int &size){
	
}

char* Message::serialize(){
	char temp_array[OUTPUT_LENGTH];
	temp_array[0] = this->col;
	temp_array[1] = this->cell;
	memcpy((void*) temp_array[2], (void*) this->command, COMMAND_LENGTH);
	return temp_array;
}
