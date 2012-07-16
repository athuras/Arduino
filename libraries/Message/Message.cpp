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

Message::Message(char col, char cell, const char* command){
	this-> col = col;
	this-> cell = cell;
	memcpy(this->command, command, COMMAND_LENGTH);
}

Message::Message(char col, char cell, int* command){
	this->col = col;
	this->cell = cell;
	char buffer[COMMAND_LENGTH];
	for (int i = 0; i < COMMAND_LENGTH; i++){
		buffer[i] = (char)(command[i]%256);
	}
	memcpy(this->command, buffer, COMMAND_LENGTH);
}

int Message::length(){
	return OUTPUT_LENGTH;
}

int Message::bodyLength(){
	return COMMAND_LENGTH;
}

void Message::serialize(char output_array[], int size){
	if (size == OUTPUT_LENGTH){
		output_array[0] = this->col;
		output_array[1] = this->cell;
		memcpy((void*) output_array[2], (void*) this->command, COMMAND_LENGTH);
	} else if (size > OUTPUT_LENGTH){
	//add in later?
	} else {
	//error
	}
}

char* Message::serialize(){
	char temp_array[OUTPUT_LENGTH];
	temp_array[0] = this->col;
	temp_array[1] = this->cell;
	memcpy((void*) temp_array[2], (void*) this->command, COMMAND_LENGTH);
	return temp_array;
}

void Message::deserialize(char* input_array, int size){
	this->col = input_array[0];
	this->cell = input_array[1];
	memcpy((void*) this->command, (void*) input_array[2], COMMAND_LENGTH);
}

void Message::empty(){
	this->col = 0;
	this->cell = 0;
	memset(this->command, 0, COMMAND_LENGTH);
}
