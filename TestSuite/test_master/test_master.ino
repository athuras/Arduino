#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>
/////////////////////////////////////////////////////
// Reference
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
const int COMMAND_LENGTH = 10;
const int CMD_BODY_LENGTH = COMMAND_LENGTH - 2;

/////////////////////////////////////////////////////
// Buffers and Such
byte fromSlaveBuffer[ RESPONSE_LENGTH ];
byte fromFrontBuffer[ FRONT_BUFFER ];

/////////////////////////////////////////////////////
// State Variables
int cycle = 0;
const int CYCLE_DELAY = 10;
const int POLL_INTERVAL = 100; // in integer multiples of 10ms cycles
bool newColumnFound   = false;
bool isInputComplete  = false;

// MAIN
void setup(){
  Serial.begin(9600);
  while (!Serial){;} // Apparently needed for Serial on Leonardo
  Wire.begin();
}

void loop(){
  serialEvent();
  if (newColumnFound){
  // hook a brother up.
  }
  if (isInputComplete){
    Serial.println("DEBUG - Shit Just Got Real.");
    byte col = fromFrontBuffer[0];
    byte cell = fromFrontBuffer[1];
    byte command = parseByteFrontCommand(fromFrontBuffer);
    byte response = 0;
    Serial.print(col); Serial.print(" "); Serial.print(cell);

    if (command == 0){ // Unlock
      Serial.println("DEBUG - Unlock . . .");
      Message msg = Message(col, cell, string_table[0]);
      messagePrint(msg);
      response = writeToSlave(msg); // anticipate block here
      if (response == 0){
        Serial.print("DEBUG - Response:\n");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
      }
      else {
        Serial.print("DEBUG - Error Unlocking - I2C Resp:");
		    Serial.println(response);
      }
    }

    else if (command == 1){ // Query Analog Sensor
      Serial.print("DEBUG - Sensor Query . . . \n");
      Message msg = Message(col, cell, string_table[1]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
        Serial.print("DEBUG - Analog Value: \n");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
		writeAnalogToFront(msg);
      }
      else {
        Serial.print("DEBUG - Error Qeurying Sensor  - I2C Resp: ");
		    Serial.println(response);
      }
    }

    else if (command == 2){ // Query Limit Switch
      Serial.print("DEBUG - Limit Switch Query . . .");
      Message msg = Message(col, cell, string_table[3]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
        Serial.print("DEBUG - Limit Switch Value: \n");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
		msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
        writeLimitToFront(msg);
      }
      else {
        Serial.print("DEBUG - Error Querying Limit Switch - I2C Resp: ");
    		Serial.println(response);
      }
    }

    else if (command == 3){ // Request Column POST (all limit switches) //probably not used
      Serial.print("DEBUG - Case 3 \n");
    }

	// Reset Address of Specified column. Whereing the cell value is the new address
    else if (command == 4){ 
	  Message msg = Message(col, cell, string_table[2]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
        Serial.print("DEBUG - New Address Assigned: ");
        Serial.print(cell); Serial.print('\n');
      } else {
        Serial.println("Shit Went Down, we can only watch now");
      }
    }
	
	//Request Cell Size
	else if (command == 5){	
		Serial.println("DEBUG - Cell Size Query . . .");
		Message msg = Message(col, cell, string_table[3]);
		messagePrint(msg);
		response = writeToSlave(msg);
		if (response == 0){
			Serial.print("DEBUG - Cell Size Value: \n");
			requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
			msg.deserialize(fromSlaveBuffer, RESPONSE_LENGTH);
			messagePrint(msg);
			writeCellTypeToFront(msg);
		} else {
			Serial.print("DEBUG - Error Querying Cell Size - I2C Resp: ");
			Serial.println(response);
		}	
	}
	
	else if (command == 9){ //helper debug command
		scan();
	}
	
    else {
      Serial.print("DEBUG - Not Valid Switch: ");
      Serial.println(command);
    }
  }

  // Periodic Default Address echo request.
  if (cycle == POLL_INTERVAL){
    Serial.print("Poll DEFAULT: ");
    cycle = 0;
    Message msg = Message(DEFAULT_ADDRESS, 0, string_table[4]);
    byte response = writeToSlave(msg);
    if (response == 0){
      requestCallBack(DEFAULT_ADDRESS, fromSlaveBuffer, RESPONSE_LENGTH);
      Serial.println("BURN THE WITCH!");
      // so now the front will KNOW there is a new column, and send the appropriate new address message
    }
    else {
      Serial.print(response); Serial.print('\n');
    }
  }
  isInputComplete = false;
  cycle++;
  delay(CYCLE_DELAY);
}

////////////////////////////////////////////////////
// Debug
void messagePrint(Message msg){
	Serial.print("Col: ");	Serial.println(msg.col);
	Serial.print("Cell: ");	Serial.println(msg.cell);
	Serial.print("Msg: ");
	for (int i = 0; i < CMD_BODY_LENGTH; i++){
		Serial.write(msg.command[i]);
	}
	Serial.print('\n');

}
////////////////////////////////////////////////////

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
  int cnt = 0;
  if (Serial.available()){
    Serial.println("Reading . . .");
  }
  while (Serial.available()){
    if (cnt < num){
      byte aux = Serial.read();
      buffer[cnt] = aux;
      cnt++;
      if (isNullTerminated && aux == 0x76){
        isInputComplete = true;
      }
    }
    if (cnt == num){
      isInputComplete = true;
      Serial.read();
    }
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
	serialFill(0, 8);
	Serial.write('\n');
}

void writeNewAddAcknowledgeToFront(Message msg){
	Serial.write('a');
	Serial.write(msg.col);
	serialFill(0,8);
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
  Serial.println((char)command[2]);
  switch (command[2]) {
    case 0x00: //unlock
      return 0;
      break;
    case 0x01: //analog query
      return 1;
      break;
    case 0x02: //limit query
      return 2;
      break;
    case 0x03: //column post
      return 3;
      break;
    case 0x04: //reset to new address
      return 4;
      break;
	case 0x05: //return size of cell
	  return 5;
	  break;
	case 0x09:
      return 9;
	  break;
    default:
      return 5;
      break;
  }
}

void serialFill(byte value, byte fillNum){
	for (byte i = 0; i < fillNum; i++){
		Serial.write(value);
	}	
}

void scan(){
	for (byte i = 1; i < 127; i++){
		Message msg = Message(i, 1, string_table[4]);
		byte response = writeToSlave(msg);
			Serial.print("At: ");
			Serial.print(i);
			Serial.print(" ");
			Serial.print(response);
			Serial.print('\n');
	}
}