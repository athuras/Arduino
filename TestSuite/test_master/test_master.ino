#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>
/////////////////////////////////////////////////////
// Reference
const char unlockcode[]  = {49, 49, 49, 49, 49, 49, 49, 49};
const char querycode[]  =   {50, 50, 50, 50, 50, 50, 50, 50};
const char new_addresscode[]  =   {51, 51, 51, 51, 51, 51, 51}; 
const char limitswitchcode[]  = {52, 52, 52, 52, 52, 52, 52, 52};

const char *string_table[] = {unlockcode, querycode, new_addresscode, limitswitchcode};

const int FRONT_BUFFER = 10;
const int SLAVE_BUFFER = 10;
const int RESPONSE_LENGTH = 10;
const int COMMAND_LENGTH = 10;
const int CMD_BODY_LENGTH = COMMAND_LENGTH - 2;

/////////////////////////////////////////////////////
// Buffers and Such
byte toSlaveBuffer[ SLAVE_BUFFER ];
byte toFrontBuffer[ FRONT_BUFFER ];
byte fromSlaveBuffer[ RESPONSE_LENGTH ];
byte fromFrontBuffer[ FRONT_BUFFER ];

/////////////////////////////////////////////////////
// State Variables
bool inLockCycle  = false;
  int lockDelay   = 100;
  byte lockedColumn = 0;
  byte lockedCell   = 0;
bool newColumnFound   = false;
bool isInputComplete  = false;
bool isSlaveResponseComplete = false;

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
    byte col = charNumToByteNum((char)fromFrontBuffer[0]);
    byte cell = charNumToByteNum((char)fromFrontBuffer[1]);
    byte command = parseFrontCommand(fromFrontBuffer);
    byte response = 0;
    Serial.print(col); Serial.print(" "); Serial.println(cell);
    if (command == 0){ // Unlock
      Serial.println("DEBUG - Unlock . . .");
      Message msg = Message(col, cell, string_table[0]);
      messagePrint(msg);
      response = writeToSlave(msg); // anticipate block here
      if (response == 0){
        Serial.print("DEBUG - Response:\n");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
        inLockCycle = true;
        lockedColumn = col;
        lockedCell = cell;
      }
      else {
        Serial.print("DEBUG - Error Unlocking\n");
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
        messagePrint(msg);
		writeAnalogToFront(msg);
      }
      else {
        Serial.print("DEBUG - Error Qeurying Sensor \n");
      }
    }
    else if (command == 2){
      Serial.print("DEBUG - Limit Switch Query . . .");
      Message msg = Message(col, cell, string_table[3]);
      messagePrint(msg);
      response = writeToSlave(msg);
      if (response == 0){
        Serial.print("DEBUG - Limit Switch Value: \n");
        requestCallBack(col, fromSlaveBuffer, RESPONSE_LENGTH);
        messagePrint(msg);
        writeLimitToFront(msg);
      }
      else {
        Serial.print("DEBUG - Error Querying Limit Switch \n");
      }
    }
    else if (command == 3){ // Request Column POST (all limit switches)
      Serial.print("DEBUG - Case 3 \n");
    }
    else {
      Serial.print("DEBUG - Not Valid Switch: ");
      Serial.println(command);
    }
  }
  isInputComplete = false;
  delay(10);
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

void limitQueryResponse(Message response){
  byte buffer[response.length()];
  response.serialize((char*)buffer, response.length());
  for (int i = 0; i < response.length(); i++){
    Serial.write(buffer[i]);
  }
  Serial.write('\n');
}

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
      char aux = (char)Serial.read();
      buffer[cnt] = aux;
      cnt++;
      if (isNullTerminated && (char)aux == '\n'){
        isInputComplete = true;
      }
    }
    if (cnt == num){
      isInputComplete = true;
      Serial.read();
    }
  }
}

byte charNumToByteNum(char c){
  if (c >= 48 && c <= 57){
    return (c - 48);
  }
}
/////////////////////////////////////////////////////
// Communication Methods
byte writeToSlave(Message msg){
  char writeBuffer[COMMAND_LENGTH];
  //msg.serialize(writeBuffer, COMMAND_LENGTH);
  Wire.beginTransmission(msg.col);
  Wire.write(msg.col);
  Wire.write(msg.cell);
  for (int i = 0; i < 8; i++){
	Wire.write(msg.command[i]);
  }
  //Wire.write((byte*)writeBuffer, COMMAND_LENGTH);
  return Wire.endTransmission();
}

void requestCallBack(byte column, byte* buffer, byte num){
  Wire.requestFrom(column, num);
  int cnt = 0;
  while (Wire.available()){
    if (cnt < num){
      buffer[cnt] = Wire.read();
      cnt++;
    }
    if (cnt == num){
      isSlaveResponseComplete = true;
      Wire.read();
    }
  }
}

void writeToFront(byte* message, byte num){
  for (byte i = 0; i < num; i++){
    Serial.write(message[i]);
  }
}

void writeLimitToFront(Message msg){
	Serial.print('l');
	Serial.print(msg.col);
	Serial.print(msg.cell);
	Serial.print(msg.command[1]);
	Serial.print(msg.command[2]);
	serialFill(0, 5);
	Serial.print('\n');
}

void writeAnalogToFront(Message msg){
	Serial.print('s');
	Serial.print(msg.col);
	Serial.print(msg.cell);
	Serial.print(msg.command[1]);
	Serial.print(msg.command[2]);
	serialFill(0, 5);
	Serial.print('\n');
}

void writeNewColToFront(Message msg){
	
}


/* Seems to not like overloading.
void writeToFront(string k){
  for (byte i = 0; i < k.length(); i++){
    Serial.write(k.charAt(i));
  }
}
*/

byte parseFrontCommand(byte* command){
  if ((char)command[2] == 'u' || (char)command[2] == 'A'){ // unlock
    return 0;
  }

  if ((char)command[2] == 's' || (char)command[2] == 'B'){ // query sensor
    return 1;
  }
  if ((char)command[2] == 'l' || (char)command[2] == 'C'){ // query limit switch
    return 2;
  }
  if ((char)command[2] == 'D'){ // spam query all limit switches
    return 3;
  }
  return 4;
}

void serialFill(byte value, byte fillNum){
	for (byte i = 0; i < fillNum; i++){
		Serial.write(value);
	}	
}