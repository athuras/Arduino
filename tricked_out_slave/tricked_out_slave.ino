// This is the slave microcontroller sketch

#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

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

//set up commands into flash memory
const byte unlockcode[]  =      {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
const byte querycode[]  =       {50, 50, 50, 50, 50, 50, 50, 50};
const byte new_addresscode[]  = {51, 51, 51, 51, 51, 51, 51, 51}; 
const byte limitswitchcode[]  = {52, 52, 52, 52, 52, 52, 52, 52};
const byte echo[]             = {53, 53, 53, 53, 53, 53, 53, 53};
const byte CELL_TYPES[]  =      {1,2,3,4}; // the length of this array relates to how many cells there are.
const byte CELL_NUM = 			sizeof(CELL_TYPES)/sizeof(byte);

// Pins are arbitrary, and should be changed depending on the requirements.

const int ANALOG_READ_PIN = A0;
const int MUX_SELECT[3] = {12,11,10};
const int DEC_SELECT[3] = {8,7};
const int DEC_ENABLE = 9;

const int PULSE_DELAY = 500;

const byte *string_table[] = 	   // change "string_table" name to suit
{   
  unlockcode, querycode, new_addresscode, limitswitchcode, echo
};

const byte DEFAULT_ADDRESS = 5; // THIS VALUE MUST NEVER BE SET TO 255.
byte current_address = 5;
const int COMMAND_LENGTH = 10;
const int RESPONSE_LENGTH = 10;
const int CMD_BODY_LENGTH = 8;
Message received_command = Message(); 
byte writeBuffer[COMMAND_LENGTH];

void setup(){
  // Resilient Address
  /*
  byte storage = EEPROM.read(0);
  if (storage == 255 || storage == DEFAULT_ADDRESS){
    current_address = DEFAULT_ADDRESS;
  }
  else {
    current_address = storage;
  }
  */
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Wire.begin(current_address);
  DEBUG_START(9600);
  pinMode(ANALOG_READ_PIN, INPUT);
  pinMode(DEC_ENABLE, OUTPUT);
  pinMode(MUX_SELECT[0], OUTPUT);
  pinMode(MUX_SELECT[1], OUTPUT);
  pinMode(MUX_SELECT[2], OUTPUT);
  pinMode(DEC_SELECT[0], OUTPUT);
  pinMode(DEC_SELECT[1], OUTPUT);

  
	  DEBUG_PRINT("Slave: ");
	  DEBUG_PRINT(current_address);
	  DEBUG_PRINT("\n");
}

void loop(){
}

void receiveEvent(int value){
  byte buffer[COMMAND_LENGTH];
  int cnt = 0;
  received_command.empty();
  while (0 < Wire.available() && cnt < COMMAND_LENGTH){
     buffer[cnt] = Wire.read();
     cnt++;
  }
  received_command.deserialize(buffer, COMMAND_LENGTH);
  DEBUG_PRINT("Recieved Message\n");
  messagePrint(received_command);

  Message msg = Message();
  if (memcmp(received_command.command, string_table[4], CMD_BODY_LENGTH) == 0){ // echo requested
	byte body[8] = {CELL_NUM, 0,0,0,0,0,0,0};
	Message msg = Message(current_address, 0, body);	
    reply(msg); 
    DEBUG_PRINTLN("Recieved Echo Request");
  } else if (memcmp(received_command.command, string_table[2], CMD_BODY_LENGTH) == 0){ // reset address code sent
	  resetAddress(received_command.cell);
	  byte body[8] = {0,0,0,0,0,0,0,0};
	  msg = Message(current_address, 0, body);
	  reply(msg);
  } else if (memcmp(received_command.command, string_table[0], CMD_BODY_LENGTH) == 0){ // unlock code sent
	  DEBUG_PRINTLN("In unlock");
      unlock(received_command.cell);
      msg = limitQuery( (int)received_command.cell);
      messagePrint(msg);
      reply( msg ); // returns status of cell opened
  } else if (memcmp(received_command.command, string_table[1], CMD_BODY_LENGTH) == 0){ // queried by master
	  DEBUG_PRINTLN("In analog query");
      msg = analogQuery(received_command.cell);
      messagePrint(msg);
      reply( msg );
  } else if (memcmp(received_command.command, string_table[2], CMD_BODY_LENGTH) == 0){  // this state shouldn't happen 
  } else if (memcmp(received_command.command, string_table[3], CMD_BODY_LENGTH) == 0){  //query limit switch
    // query limit switch status.
	DEBUG_PRINTLN("In limit switch query");
    msg = limitQuery(received_command.cell);
    messagePrint(msg);
    reply(msg); 
  }
}

void requestEvent(){
	DEBUG_PRINTLN("Writing to master");
	for (int i = 0; i < RESPONSE_LENGTH; i++){
		DEBUG_WRITE(writeBuffer[i]);
	}
	Wire.write(writeBuffer, RESPONSE_LENGTH);
}


void messagePrint(Message msg){
  DEBUG_PRINT("Message: ");
  DEBUG_WRITE(msg.col);
  DEBUG_WRITE(msg.cell);
  for (int i = 0; i < 8; i++){
	DEBUG_WRITE(msg.command[i]);
  }
  DEBUG_PRINT('\n');
  return;

}

// To avoid embarrasing mass unlock scenarios, pulse timing should be tuned to relay.
void pulse(int pin){
  digitalWrite(pin, HIGH);
  delay(PULSE_DELAY);
  digitalWrite(pin, LOW);
  return;
}

void pulseLow(int pin){
  digitalWrite(pin, LOW);
  delay(PULSE_DELAY);
  digitalWrite(pin, HIGH);
  return;
}

void resetAddress(byte address){
  //EEPROM.write(0, address); 
  current_address = address;
  Wire.onReceive(receiveEvent);
  Wire.begin(address);
  return;
}

void unlock(byte cell){
	bool validCell = true;
	//assuming active low decoder
	digitalWrite(DEC_ENABLE, HIGH); //disables decoder, all outputs hot
	switch (cell){
		case 1:		//output 0 selected
			digitalWrite(DEC_SELECT[0], LOW);
			digitalWrite(DEC_SELECT[1], LOW);
			break;
		case 2:		//output 1 selected
			digitalWrite(DEC_SELECT[0], HIGH);
			digitalWrite(DEC_SELECT[1], LOW);
			break;
		case 3:		//output 2  selected
			digitalWrite(DEC_SELECT[0], LOW);
			digitalWrite(DEC_SELECT[1], HIGH);
			break;
		case 4:		//output 3 selected
			digitalWrite(DEC_SELECT[0], HIGH);
			digitalWrite(DEC_SELECT[1], HIGH);
			break;
		default:
			DEBUG_PRINTLN("Invalid cell for limit switch query");
			validCell = false;
	}
	if (validCell){
		pulseLow(DEC_ENABLE);
		digitalWrite(DEC_ENABLE, HIGH);
	}
}

Message limitQuery(byte cell){
	bool validCell = true;
	digitalWrite(MUX_SELECT[0], LOW);
	switch (cell){
		case 1:
			digitalWrite(MUX_SELECT[1], LOW);
			digitalWrite(MUX_SELECT[2], LOW);
			break;
		case 2:
			digitalWrite(MUX_SELECT[1], LOW);
			digitalWrite(MUX_SELECT[2], HIGH);
			break;
		case 3:
			digitalWrite(MUX_SELECT[1], HIGH);
			digitalWrite(MUX_SELECT[2], LOW);
			break;
		case 4:
			digitalWrite(MUX_SELECT[1], HIGH);
			digitalWrite(MUX_SELECT[2], HIGH);
			break;
		default:
			DEBUG_PRINTLN("Invalid cell for limit switch query");
			validCell = false;
	}
	if (validCell){
		byte type = CELL_TYPES[cell - 1];
		int reading = analogRead(ANALOG_READ_PIN);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);
	} else {
		byte body[] = {255, 255, 255, 255, 255, 255, 255, 255};
		return Message(current_address, cell, body);
	}
}

Message analogQuery(byte cell){
	bool validCell = true;
	digitalWrite(MUX_SELECT[1], LOW);
	switch (cell){
		case 1:
			digitalWrite(MUX_SELECT[1], LOW);
			digitalWrite(MUX_SELECT[2], LOW);
			break;
		case 2:
			digitalWrite(MUX_SELECT[1], LOW);
			digitalWrite(MUX_SELECT[2], HIGH);
			break;
		case 3:
			digitalWrite(MUX_SELECT[1], HIGH);
			digitalWrite(MUX_SELECT[2], LOW);
			break;
		case 4:
			digitalWrite(MUX_SELECT[1], HIGH);
			digitalWrite(MUX_SELECT[2], HIGH);
			break;
		default:
			DEBUG_PRINTLN("Invalid cell for analog sensor query");
			validCell = false;
	}
	if (validCell){
		byte type = CELL_TYPES[cell - 1];
		int reading = analogRead(ANALOG_READ_PIN);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);
	} else {
		byte body[] = {255, 255, 255, 255, 255, 255, 255, 255};
		return Message(current_address, cell, body);
	}
}

void reply(Message msg){
	writeBuffer[0] = msg.col;
	writeBuffer[1] = msg.cell;
	for (byte i = 0; i < RESPONSE_LENGTH; i++){
		writeBuffer[i+2] = msg.command[i];
	}	
  return;
}
