#include <Arduino.h>

#ifndef MESSAGE_H
#define MESSAGE_H

struct Message{
	public:
	//constructor
	Message();
	Message(byte col, byte cell, byte* command);
	Message(byte col, byte cell, const byte* command);
	Message(byte col, byte cell, int* command);

	
	//serialize to string for Wire.write(byte[]);
	void serialize(byte array[], int size);
	byte* serialize();
	void deserialize(byte* array, int size);
	void empty();
	int length();
	int bodyLength();
	
	//contents
	byte col;
	byte cell;
	byte command[8];

};

#endif
