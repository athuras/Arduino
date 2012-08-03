// This is the slave microcontroller sketch

#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

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
const byte CELL_TYPES[]  =      {2,2}; // the length of this array relates to how many cells there are.
const byte CELL_NUM = 			sizeof(CELL_TYPES)/sizeof(byte);

// Pins are arbitrary, and should be changed depending on the requirements.

const int LOCK_PULSE_PIN = 10;	//pin used to pulse decoder enable
const int LIMIT1_IN = A0; // must be analog in
const int LIMIT2_IN = A1; // must be analog in
const int ANALOG_IN = A3;

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

const byte MUX_1 = 5;
const byte MUX_2 = 6;
const byte DEC_1 = 7;
const byte DEC_2 = 8;
const byte DEC_CONTROL = 9;

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
  pinMode(LIMIT1_IN, INPUT);
  pinMode(LIMIT2_IN, INPUT);
  pinMode(ANALOG_IN, INPUT);
  
  
  pinMode(MUX_1, OUTPUT);
  pinMode(MUX_2, OUTPUT);
  pinMode(DEC_1, OUTPUT);
  pinMode(DEC_2, OUTPUT);
  pinMode(DEC_CONTROL, OUTPUT);
  digitalWrite(DEC_CONTROL, HIGH);
  
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
  DEBUG_PRINTLN("Pulsing: ");
  DEBUG_PRINTLN(pin);
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
	if (cell == 1){
		pulse(DEC_1);
	}
	if (cell == 2){
		pulse(DEC_2);
	}
}

Message limitQuery(byte cell){
	if (cell == 1){
		byte type = CELL_TYPES[cell - 1];
		digitalWrite(MUX_1, LOW);
		digitalWrite(MUX_2, LOW);
		int reading = analogRead(LIMIT1_IN);
		DEBUG_PRINTLN(reading);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);
	}
	if (cell == 2){
		byte type = CELL_TYPES[cell - 1];
		digitalWrite(MUX_1, HIGH);
		digitalWrite(MUX_2, LOW);
		int reading = analogRead(LIMIT2_IN);
		DEBUG_PRINTLN(reading);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);	
	}
}

Message analogQuery(byte cell){
	if (cell == 1){
		byte type = CELL_TYPES[cell - 1];
		digitalWrite(MUX_1, LOW);
		digitalWrite(MUX_2, HIGH);
		int reading = analogRead(ANALOG_IN);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);
	}
	if (cell == 2){
		byte type = CELL_TYPES[cell - 1];
		digitalWrite(MUX_1, HIGH);
		digitalWrite(MUX_2, HIGH);
		int reading = analogRead(ANALOG_IN);
		byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
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
