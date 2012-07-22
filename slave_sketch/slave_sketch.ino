// This is the slave microcontroller sketch

#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

//set up commands into flash memory
const byte unlockcode[]  =      {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
const byte querycode[]  =       {50, 50, 50, 50, 50, 50, 50, 50};
const byte new_addresscode[]  = {51, 51, 51, 51, 51, 51, 51, 51}; 
const byte limitswitchcode[]  = {52, 52, 52, 52, 52, 52, 52, 52};
const byte echo[]             = {53, 53, 53, 53, 53, 53, 53, 53};
const byte CELL_TYPES[]  =      {1,2,3,4,5,6,7,8,9,10}; // the length of this array relates to how many cells there are.
const byte CELL_NUM = 			sizeof(CELL_TYPES)/sizeof(byte);

// Pins are arbitrary, and should be changed depending on the requirements.

const int CELL_COUNT = 10;
const int CONTROL_SIZE = 4 + 1; // the last '1' is for the limit switch
const int DEC_OUT = 10;
const int MUX_IN = 6; // must be analog in
const int muxSelectPins[CONTROL_SIZE] = {1,2,3,4,5}; // the 5th pin is to toggle limit switch
const int decodeControlPins[CONTROL_SIZE - 1] = {1,2,3,4};
const byte LIMIT_SELECT_PIN = 4;

const int PULSE_DELAY = 1000;

const byte *string_table[] = 	   // change "string_table" name to suit
{   
  unlockcode, querycode, new_addresscode, limitswitchcode, echo
};

const byte DEFAULT_ADDRESS = 5; // THIS VALUE MUST NEVER BE SET TO 255.
byte current_address = 20;
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
  Serial.begin(9600);
  while (!Serial){;}
  //Serial.println(storage);
  pinMode(MUX_IN, INPUT);
  pinMode(DEC_OUT, OUTPUT);
  for ( int i = 0; i < CONTROL_SIZE - 1; i++){
    pinMode(muxSelectPins[i], OUTPUT);
  }
  for ( int i = 0; i < CONTROL_SIZE - 2; i++){ // note the decoder has one-fewer control pin
    pinMode(decodeControlPins[i], OUTPUT);
  }
  Serial.print("Slave: ");
  Serial.print(current_address);
  Serial.print("\n");
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
  Serial.print("Recieved Message\n");
  messagePrint(received_command);

  Message msg = Message();
  if (memcmp(received_command.command, string_table[4], CMD_BODY_LENGTH) == 0){ // echo requested
   reply(received_command); 
   Serial.println("Recieved Echo Request");
  } else if (memcmp(received_command.command, string_table[2], CMD_BODY_LENGTH) == 0){ // reset address code sent
    resetAddress(received_command.cell);
  } else if (memcmp(received_command.command, string_table[0], CMD_BODY_LENGTH) == 0){ // unlock code sent
	Serial.println("In unlock");
      unlock(received_command.cell);
      msg = query( (int)received_command.cell);
      messagePrint(msg);
      reply( msg ); // returns status of cell opened
  } else if (memcmp(received_command.command, string_table[1], CMD_BODY_LENGTH) == 0){ // queried by master
	Serial.println("In analog query");
      msg = analogQuery(received_command.cell);
      messagePrint(msg);
      reply( msg );
  } else if (memcmp(received_command.command, string_table[2], CMD_BODY_LENGTH) == 0){  // this state shouldn't happen 
  } else if (memcmp(received_command.command, string_table[3], CMD_BODY_LENGTH) == 0){  //query limit switch
    // query limit switch status.
	Serial.println("In limit switch query");
    msg = limitQuery(received_command.cell);
    messagePrint(msg);
    reply(msg); 
  }
}

void requestEvent(){
	Serial.println("Writing to master");
	Wire.write(writeBuffer, RESPONSE_LENGTH);
}


void messagePrint(Message msg){
  Serial.print("Message: ");
  Serial.write(msg.col);
  Serial.write(msg.cell);
  for (int i = 0; i < 8; i++){
	Serial.write(msg.command[i]);
  }
  Serial.print('\n');
  return;
}

// One-based. i.e. muxSelect( 1 ) selects the first (zeroith) mux in.
void muxSelect(int id){ 
	int aux = id -1;
	for (int k = CONTROL_SIZE - 1; k >= 0; k--){
		int size = (int) pow( (double)2, (double)k);
		if (aux >= size){
			digitalWrite(muxSelectPins[k], HIGH);
			aux -= size;
		} else {
			digitalWrite(muxSelectPins[k], LOW);
		}
	}
 return;
}
// Again, one-based. decSelect(1) selects the first (zeroith) dec out.
void decSelect(int id){
  int aux = id - 1;
  for ( int k = CONTROL_SIZE -1; k >=0; k--){
    int a = aux % (int) pow(2,k);
    if ( a == aux ){
      digitalWrite(decodeControlPins[k], LOW);
    }
    else {
      digitalWrite(decodeControlPins[k], HIGH);
      aux = aux - a;
    }
  }
  return;
}

// To avoid embarrasing mass unlock scenarios, pulse timing should be tuned to relay.
void pulse(int pin){
  pinMode(pin,OUTPUT);
  digitalWrite(pin, HIGH);
  delay(PULSE_DELAY);
  digitalWrite(pin, LOW);
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
  if (cell == 0){
       //this is an error state 
  } else {
    decSelect( (int) cell );
    pinMode(DEC_OUT,OUTPUT);
    pulse(DEC_OUT);
  }
  return;
}

Message limitQuery(byte cell){
	digitalWrite(muxSelectPins[LIMIT_SELECT_PIN], HIGH);
	return query(cell);
}

Message analogQuery(byte cell){
	digitalWrite(muxSelectPins[LIMIT_SELECT_PIN], LOW);
	return query(cell);
}

Message query(byte cell){
 int reading = LOW;
  if (cell == 0){
    int proxy_cell_count[] = {CELL_COUNT, 0,0,0,0,0,0,0};
    return Message(current_address, 0, proxy_cell_count);
     // return the number of consecutaive cells to master. master should then sequentially query each cell.
  } else {
	if (cell >= CELL_NUM){
		byte body[] = {0,0,0,0,0,0,0,0};
		return Message(current_address, cell, body);
	} else if (cell < CELL_NUM){
		byte type = CELL_TYPES[cell - 1];
		muxSelect( (int) cell );
		pinMode(MUX_IN, INPUT);
		reading = analogRead(MUX_IN);
		  byte body[] = {type, highByte(reading), lowByte(reading), 0, 0, 0, 0, 0};
		return Message(current_address, cell, body);
	}
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
