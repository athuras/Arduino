#include "Message.h"

Message::Message(){
	col = 0;
	cell = 0;
	command = NULL;
}

Message::Message(byte col, byte cell, byte[] command)
{
	this->col = col;
	this->cell = cell;
	for (int i = 0; i < 8; i++){
		this->command[i] = command[i];
	}
}

char[] serialize(){

}
