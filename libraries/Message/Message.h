#ifndef MESSAGE_H
#define MESSAGE_H

struct Message{
	public:
	//constructor
	Message();
	Message(char col, char cell, char* command);
	Message(char col, char cell, const char* command);
	Message(char col, char cell, int* command);

	
	//serialize to string for Wire.write(char[]);
	void serialize(char array[], int size);
	char* serialize();
	void deserialize(char* array, int size);
	void empty();
	int length();
	int bodyLength();
	
	//contents
	char col;
	char cell;
	char command[8];

};

#endif
