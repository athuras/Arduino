#include "Message.h"

Message::Message(){
	col = 0;
	cell = 0;
	command = NULL;
}

Message::Message(char col, char cell, char[] command)
{
	this->col = col;
	this->cell = cell;
	for (int i = 0; i < 8; i++){
		this->command[i] = command[i];
	}
}

char[] serialize(){

}
