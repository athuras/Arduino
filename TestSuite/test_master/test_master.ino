#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>

//uncomment the line below to enable debug
//#define DEBUG
#ifdef DEBUG
	#define DEBUG_PRINT(x) 		Serial.print(x)
	#define DEBUG_PRINTLN(x) 	Serial.println(x)
	#define DEBUG_WRITE(x) 		Serial.write(x)
	#define DEBUG_START(x); \
			Serial.begin(x); \
			while (!Serial){;}
#else
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINTLN(x)
	#define DEBUG_WRITE(x)
	#define DEBUG_START(x);
#endif

/////////////////////////////////////////////////////
// Reference strings used for comprehending messages
// This list should (read 'must') be consistent across the whole platform
const byte unlockcode[]   =   {49, 49, 49, 49, 49, 49, 49, 49};
const byte querycode[]    =   {50, 50, 50, 50, 50, 50, 50, 50};
const byte new_addresscode[]  =   {51, 51, 51, 51, 51, 51, 51, 51};
const byte limitswitchcode[]  =   {52, 52, 52, 52, 52, 52, 52, 52};
const byte echo[] = {53, 53, 53, 53, 53, 53, 53, 53};

const byte *string_table[] = {unlockcode, querycode, new_addresscode, limitswitchcode, echo};

const byte DEFAULT_ADDRESS = 5; // THIS SHOULD NEVER BE SET TO 255
const int FRONT_BUFFER = 10;
const int SLAVE_BUFFER = 10;
const int RESPONSE_LENGTH = 10;
const int COMMAND_LENGTH = 10; //This is a parameter used in Message, number of bytes contained in a Message
const int CMD_BODY_LENGTH = COMMAND_LENGTH - 2; //The first two bytes are used as addresses

/////////////////////////////////////////////////////
// Buffers for reading complete messages from both input ends (front, slaves)
byte fromSlaveBuffer[ RESPONSE_LENGTH ];
byte fromFrontBuffer[ FRONT_BUFFER ];

/////////////////////////////////////////////////////
// State Variables
int cycle = 0;
const int CYCLE_DELAY = 10; // wait time betwen main loop cycles,
const int POLL_INTERVAL = 100; // The number of loop cycles to wait before polling the default address for new slaves
bool newColumnFound   = false;
bool isInputComplete  = false;
bool ledon = false;

// MAIN
void setup(){
  Serial.begin(9600);
  while (!Serial){;} // Apparently needed for Serial on Leonardo, we don't know why, and this may introduce problems
  Wire.begin();
}

void loop(){
  serialEvent();

  if (isInputComplete){
    if (DEBUG) {Serial.println("DEBUG - Shit Just Got Real.");}
    byte col = fromFrontBuffer[1];
    byte cell = fromFrontBuffer[2];
    byte command = parseByteFrontCommand(fromFrontBuffer);
    byte response = 0;

    if (command == 0){ // Unlock
	  DEBUG_PRINTLN("DEBUG - Unlock . . .");
      Message msg = Message(col, cell, string_table[0]);
      messagePrint(msg);
      response = writeToSlave(msg); // anticipate block here
      if (response == 0){
		DEBUG_PRINTLN("DEBUG - RESPONSE: ");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);		
      }
      else {
		DEBUG_PRINT("DEBUG - Error Unlocking - I2C Resp:");
		DEBUG_PRINT(response);
		// unlock cycle, keeps trying to get state of the box to determine when the door closes
		// this sequence could be moved to the front-end, but the choice is arbitrary at this point
		/*
		delay(50);
		bool doorClosed = false;
		while(!doorClosed){
			msg = Message(col, cell, string_table[3]);
			response = writeToSlave(msg);
			if (response == 0){
				requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
				msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
				doorClosed = isDoorClosed(msg);
				writeLimitToFront(msg);
			} else {
				//fucking terrible error state
				if (DEBUG) {
					Serial.println("All hope is lost");
					Serial.println(response);
				}
			}
		}
		*/
      }
    }

    else if (command == 1){ // Query Analog Sensor
	  DEBUG_PRINTLN("DEBUG - Sensor Query . . . ");
      Message msg = Message(col, cell, string_table[1]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
		DEBUG_PRINTLN("DEBUG - Analog Value: ");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
	      msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
		    writeAnalogToFront(msg);
      }
      else {
		DEBUG_PRINT(DEBUG- Error Querying Sensor - I2C Resp: ");
		DEBUG_PRINTLN(response);
		}
      }
    

    else if (command == 2){ // Query Limit Switch
	  DEBUG_PRINTLN("DEBUG - Limit Switch Query . . .");
      Message msg = Message(col, cell, string_table[3]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
		DEBUG_PRINTLN("DEBUG - Limit Switch Value: ");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
        writeLimitToFront(msg);
      }
      else {
		DEBUG_PRINT("DEBUG - Error Unlocking - I2C Resp:");
		DEBUG_PRINT(response);
      }
    }

    else if (command == 3){ // Request Column POST (all limit switches) //probably not used
	  DEBUG_PRINTLN("DEBUG - Case 3");
    }

	// Reset Address of Specified column. Whereing the cell value is the new address
    else if (command == 4){
	  Message msg = Message(col, cell, string_table[2]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
		requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
		writeNewAddAcknowledgeToFront(msg);
		DEBUG_PRINT("DEBUG - New Address Assigned: ");
		DEBUG_PRINTLN(CELL);
      } else {
		DEBUG_PRINTLN("Shit Went Down, we can only watch now");
      }
    }

	//Request Cell Size
	else if (command == 5){	
		DEBUG_PRINTLN("DEBUG - Cell Size Query . . .");
		Message msg = Message(col, cell, string_table[3]);
		if (DEBUG){ messagePrint(msg);}
		response = writeToSlave(msg);
		if (response == 0){
			DEBUG_PRINTLN("DEBUG - Cell Size Values: ");
			requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
			msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
			messagePrint(msg);
			writeCellTypeToFront(msg);
		} else {
			DEBUG_PRINT("DEBUG - Error Querying Cell Size - I2C Resp: ");
			DEBUG_PRINTLN(response);
		}	
	}

	else if (command == 9){ // helper debug command
		scan();
	}
    else {
		DEBUG_PRINT("DEBUG - Not Valid Command: ");
		DEBUG_PRINTLN(command);
    }
  

  // Periodic Default Address echo request.
  if (cycle == POLL_INTERVAL){
	DEBUG_PRINT("Polling Default: ");
    cycle = 0;
    Message msg = Message(DEFAULT_ADDRESS, 0, string_table[4]);
    byte response = writeToSlave(msg);
	DEBUG_PRINTLN(response);
    if (response == 0){
	  DEBUG_PRINTLN("New column found at default address");
      requestCallBack(DEFAULT_ADDRESS, fromSlaveBuffer, RESPONSE_LENGTH);
	  msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
	  writeNewColToFront(msg);
    }
    else {
    }
  }
  isInputComplete = false;
  cycle++;
  delay(CYCLE_DELAY);
  }
}

////////////////////////////////////////////////////
// Debug
void messagePrint(Message msg){
	DEBUG_PRINT("Col: ");	DEBUG_PRINTLN(msg.col);
	DEBUG_PRINT("Cell: ");	DEBUG_PRINTLN(msg.cell);
	DEBUG_PRINT("Msg: ");
	for (int i = 0; i < CMD_BODY_LENGTH; i++){
		DEBUG_WRITE(msg.command[i]);
	}
	DEBUG_PRINT('\n');	
}
////////////////////////////////////////////////////
// Serial Events
void serialEvent(){
  readByteArrayFromSerial(fromFrontBuffer, FRONT_BUFFER);
}
void readByteArrayFromSerial(byte* buffer, byte num){
  readArrayFromSerial(buffer, num, false);
}
void readCharArrayFromSerial(byte* buffer, byte num){
  readArrayFromSerial(buffer, num, true);
}

void readArrayFromSerial(byte* buffer, byte num, bool isNullTerminated){
  if (Serial.available()){
	DEBUG_PRINTLN("Reading...");
  }
  if (Serial.available() >= 10){    Serial.readBytes((char*)buffer, 10);
    while (Serial.available()){
     Serial.read();
    }
    isInputComplete = true;
  }
}

/////////////////////////////////////////////////////
// Communication Methods
byte writeToSlave(Message msg){
  Wire.beginTransmission(msg.col);
  Wire.write(msg.col);
  Wire.write(msg.cell);
  for (int i = 0; i < 8; i++){
	Wire.write(msg.command[i]);
  }
  return Wire.endTransmission();
}

void requestCallBack(byte column, byte* buffer, byte num){
  delay(100);
  Wire.requestFrom(column, num);
  int cnt = 0;
  while (Wire.available()){
    if (cnt < num){
      buffer[cnt] = Wire.read();
      cnt++;
    }
    if (cnt == num){
      Wire.read();
    }
  }
}

void writeLimitToFront(Message msg){
	Serial.write('l');
	Serial.write(msg.col);
	Serial.write(msg.cell);
	Serial.write(msg.command[1]);
	Serial.write(msg.command[2]);
	serialFill(0, 5);
	Serial.write('\n');
}

void writeAnalogToFront(Message msg){
	Serial.write('s');
	Serial.write(msg.col);
	Serial.write(msg.cell);
	Serial.write(msg.command[1]);
	Serial.write(msg.command[2]);
	serialFill(0, 5);
	Serial.write('\n');
}

void writeNewColToFront(Message msg){
	Serial.write('n');
	Serial.write(DEFAULT_ADDRESS);
	Serial.write(msg.command[0]);
	serialFill(0, 7);
	Serial.write('\n');
}

void writeNewAddAcknowledgeToFront(Message msg){
	Serial.write('a');
	Serial.write(msg.col);
	Serial.write(msg.cell);
	Serial.write(msg.command[0]);
	serialFill(0,7);
	Serial.write('\n');
}

void writeCellTypeToFront(Message msg){
	Serial.write('t');
	Serial.write(msg.col);
	Serial.write(msg.cell);
	Serial.write(msg.command[0]);
	serialFill(0,6);
	Serial.write('\n');
}

byte parseByteFrontCommand(byte* command){
	byte c = command[0];
	if (c == 'U' || c == 0){	//unlock
		return 0;
	} else if (c == 'S' || c == 1){	//analog query
		return 1;
	} else if (c == 'L' || c == 2){	//limit switch query
		return 2;
	} else if (c == 'N' || c == 3){	//column post
		return 3;
	} else if (c == 'A' || c == 4){	//set new address
		return 4;
	} else if (c == 'T' || c == 5){	//cell size
		return 5;
	} else if (c == 9){	//helper scan function, remove in future
		return 9;
	} else {	//default
		return 6;
	}
}

void serialFill(byte value, byte fillNum){
	for (byte i = 0; i < fillNum; i++){
		Serial.write(value);
	}
}
// Queries each available I2C address with an echo, prints ack.
void scan(){
	for (byte i = 1; i < 127; i++){
		Message msg = Message(i, 1, string_table[4]);
		byte response = writeToSlave(msg);
			DEBUG_PRINT("At: ");
			DEBUG_PRINT(i);
			DEBUG_PRINT(" ");
			DEBUG_PRINT(response);
			DEBUG_PRINT('\n');
	}
}

bool isDoorClosed(Message msg){
	//if active low
	if (word(msg.command[1], msg.command[2]) < 100){
		DEBUG_PRINTLN("Door Closed");
		return true;
	} else {
		DEBUG_PRINTLN("Door open");
		return false;
	}
	/*
	For active high case
	if (word(msg.command[1], msg.command[2]) > 900){
		return true
	}
	return false;
	*/
}
