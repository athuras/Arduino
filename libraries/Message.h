#ifndef MESSAGE_H
#define MESSAGE_H

struct Message{
	public:
	//constructor
	Message();
	Message(char col, char cell, char[] command);
	
	//serialize to string for Wire.write(char[]);
	char[] serialize();
	
	//contents
	char col;
	char cell;
	char[8] command;

};

#endif
