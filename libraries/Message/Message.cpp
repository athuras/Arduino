#include "Message.h"
#include "string.h"

const int COMMAND_LENGTH = 8;
const int OUTPUT_LENGTH = 10;

Message::Message(){
	col = 0;
	cell = 0;
	memset(this->command, 0, COMMAND_LENGTH);
}

Message::Message(byte col, byte cell, byte* command)
{
	this-> col = col;
	this-> cell = cell;
	memcpy(this->command, command, COMMAND_LENGTH);
}

Message::Message(byte col, byte cell, const byte* command){
	this-> col = col;
	this-> cell = cell;
	memcpy(this->command, command, COMMAND_LENGTH);
}

Message::Message(byte col, byte cell, int* command){
	this->col = col;
	this->cell = cell;
	byte buffer[COMMAND_LENGTH];
	for (int i = 0; i < COMMAND_LENGTH; i++){
		buffer[i] = (byte)(command[i]%256);
	}
	memcpy(this->command, buffer, COMMAND_LENGTH);
}

int Message::length(){
	return OUTPUT_LENGTH;
}

int Message::bodyLength(){
	return COMMAND_LENGTH;
}

void Message::serialize(byte output_array[], int size){
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

byte* Message::serialize(){
	byte temp_array[OUTPUT_LENGTH];
	temp_array[0] = this->col;
	temp_array[1] = this->cell;
	memcpy((void*) temp_array[2], (void*) this->command, COMMAND_LENGTH);
	return temp_array;
}

void Message::deserialize(byte* input_array, int size){
	this->col = input_array[0];
	this->cell = input_array[1];
	for(int i = 0; i < COMMAND_LENGTH; i++){
		this->command[i] = input_array[i+2];
	}
}

void Message::empty(){
	this->col = 0;
	this->cell = 0;
	memset(this->command, 0, COMMAND_LENGTH);
}
