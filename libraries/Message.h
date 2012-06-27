#ifndef MESSAGE_H
#define MESSAGE_H

struct Message{
	public:
	//constructor
	Message();
	Message(byte col, byte cell, byte[] command);
	
	//serialize to string for Wire.write(char[]);
	char[] serialize();
	
	//contents
	byte col;
	byte cell;
	byte[8] command;

};

#endif
