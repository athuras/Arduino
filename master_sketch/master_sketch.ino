
#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>


//set up commands into flash memory
prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51} ; 
prog_char limitswitchcode[] PROGMEM = {52, 52, 52, 52, 52, 52, 52, 52};

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{unlockcode, querycode, new_addresscode, limitswitchcode};

bool isInputComplete = false;
bool isSlaveResponseComplete = false;
byte toSlaveBuffer[10];
byte toFrontBuffer[10];
byte fromSlaveBuffer[10];
byte fromFrontBuffer[10];
byte slaveResponseLength = 10;

bool inLockCycle = false;
int lockDelay = 100;
byte lockedColumn = 0;
byte lockedCell = 0;

bool newColumnFound = false;
byte defaultAddress = 99; //pull this out later

void setup(){
  Serial.begin(9600);
  Wire.begin();
  newColumnFound = false;
  inLockCycle = false;
  isInputComplete = false;
}

void loop(){
	serialEvent();
	if (newColumnFound){
		
	}
	if (inLockCycle){
		Message msg = Message(lockedColumn, lockedCell, string_table[3]);
		byte response = writeToSlave(msg);
		byte limitSwitchStatus = 0;
		byte analogQuery[msg.bodyLength()];
		if (response == 0){
			requestCallBack(lockedColumn, fromSlaveBuffer, slaveResponseLength);
			limitSwitchStatus = fromSlaveBuffer[3];
			if (limitSwitchStatus = 1){ //lock is closed
				msg = Message(lockedColumn, lockedCell, string_table[1]);
				if (writeToSlave(msg) == 0){
					requestCallBack(lockedColumn, fromSlaveBuffer, slaveResponseLength);
					memcpy((void*)analogQuery, (void*)fromSlaveBuffer[2], msg.bodyLength());
					//print to serial to tell front end
					//send both a 'it's closed' signal
					//and a signal with the analog query
				} else {
					//error crap
				}
				inLockCycle = false;
				lockedColumn = 0;
				lockedCell = 0;
			} else { //lock is still open
				//do nothing
			}
		}
	}
	else {
		if (isInputComplete){
			Serial.println("DEBUG: Input complete, now parsing");
			byte col = charNumToByteNum((char)fromFrontBuffer[0]);
			byte cell = charNumToByteNum((char)fromFrontBuffer[1]);
			byte command = parseFrontEndCommand(fromFrontBuffer); //wrap this into an enum
			byte response = 0;
			if (command == 0){ //unlock
				Serial.println("DEBUG: Unlock command recieved, sending");
				Message msg = Message(col, cell, string_table[0]);  
				Serial.print("MSG: ");
				msgDebug(msg);
				Serial.print('\n');
				response = writeToSlave(msg);
				if (response == 0){
					Serial.print("DEBUG: Response: ");
					Serial.println(response);
					requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
					inLockCycle = true;
					lockedColumn = col;
					lockedCell = cell;
				} else {
					Serial.print(F("Error unlocking: Col: "));
					Serial.print(col);
					Serial.print(F(" Cell: "));
					Serial.print(cell);
					Serial.print(F(" Resp: "));
					Serial.println(response);
				}
			}
			if (command == 1){ //query analog sensor
				Serial.println("DEBUG: Analog query cmd recieved, sending");
				Message msg = Message(col, cell, string_table[1]); 
				response = writeToSlave(msg);
				Serial.print("MSG: ");
				msgDebug(msg);
				Serial.print('\n');
				if (response == 0){
					Serial.print("DEBUG: Response: ");
					Serial.println(response);
					requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
				} else {
					Serial.print(F("Error analog query: Col: "));
					Serial.print(col);
					Serial.print(F(" Cell: "));
					Serial.print(cell);
					Serial.print(F(" Resp: "));
					Serial.println(response);
				}
			}
			if (command == 2){ //query limit switch							
				Serial.println("DEBUG: limit query cmd recieved, sending");
				Message msg = Message(col, cell, string_table[3]); 
				response = writeToSlave(msg);
				Serial.print("MSG: ");
				msgDebug(msg);
				Serial.print('\n');
				if (response == 0){
					Serial.print("DEBUG: Response: ");
					Serial.println(response);
					requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
				} else {
					Serial.print(F("Error limit switch query: Col: "));
					Serial.print(col);
					Serial.print(F(" Cell: "));
					Serial.print(cell);
					Serial.print(F(" Resp: "));
					Serial.println(response);	
				}      
			}
			if (command == 3){ //query all limit switches
				Message msg = Message(col, 0, string_table[3]); 
				response = writeToSlave(msg);
				if (response == 0){
					requestCallBack(col, fromSlaveBuffer, slaveResponseLength);
				} else {
					printError(msg, response);
				}      
			}
			isInputComplete = false;
		//examines the default address location
		} else {
			//sends a dummy query
			Message msg = Message(defaultAddress, 0, string_table[0]);
			if (writeToSlave(msg) == 0){
				newColumnFound = true;
				requestCallBack(defaultAddress, fromSlaveBuffer, slaveResponseLength);
				
				//need to query front end for free address
				//request response with long enough length
				//the response will contact locker information
				//send that locker information upstream to front-end
			} else {
				//this is the usual case: no new device
				//no action required
			}
		}
	}	
   delay(10);
}

byte writeToSlave(Message msg){
  int length = 10; //this is dummied
  char writeBuffer[length];
  msg.serialize(writeBuffer, length);
  	Wire.beginTransmission(msg.col);
	Wire.write((byte*)writeBuffer, length);
	return Wire.endTransmission();
}

void requestCallBack(byte column, byte* buffer, byte length){
  Wire.requestFrom(column, length);
  int cnt = 0;
  while (Wire.available()){
    if (cnt < length){
      char inChar = Wire.read();
      buffer[cnt] = inChar;
      cnt++;
    } 
    if (cnt == length){
      isSlaveResponseComplete = true;
      Wire.read(); 
    }
  } 
}

byte charNumToByteNum(char c){
  if (c >=48 && c <= 57){
     return (c - 48); 
  } 
}

void serialEvent(){
  readByteArrayFromSerial(fromFrontBuffer, 10);
}

//helper function to read null terminated character arrays 
//null terminator is included in the length
void readCharArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, true);
}

//helper function to read byte array of length length
void readByteArrayFromSerial(byte* buffer, byte length){
	readArrayFromSerial(buffer, length, false);
}

//reads request length from serial into specified 8bit array
//after requested length is read, continues to read off the line until Serial is empty
void readArrayFromSerial(byte* buffer, byte length, boolean isNullTerminated){
	int cnt = 0;
	if (Serial.available()){
		Serial.println("reading from serial");
	}
	while (Serial.available()){
		if (cnt < length){
			char temp = (char)Serial.read();
			buffer[cnt] = temp;
			cnt++;
			if (isNullTerminated && (char)temp == '\n'){
				isInputComplete = true;
			}
		}
		if (cnt == length){
			isInputComplete = true;
			Serial.read();
		}
	}
}	

void limitQueryResponse(Message response){
	byte buffer[response.length()];
	response.serialize((char*)buffer, response.length());
	for (int i = 0; i < response.length(); i++){
		Serial.write(buffer[i]);
	}
	Serial.write('\n');
}

void writeToFront(byte* message, byte length){
	for (byte i = 0; i < length; i++){
		Serial.write(message[i]);
	}
}

void writeToFront(String str){
	for (byte i = 0; i < str.length(); i++){
		Serial.write(str.charAt(i));
	}
}

byte parseFrontEndCommand(byte* command){
	if ((char)command[2] == 'A'){
		return 0;
	}
	if ((char)command[2] == 'B'){
		return 1;
	}
	if ((char)command[2] == 'C'){
		return 2;
	}
	if ((char)command[2] == 'D'){
		return 3;
	}
}

void printError(Message msg, byte response){
	Serial.print("Error: ");
	Serial.print("Col: ");
	Serial.print(msg.col);
	Serial.print("Cell: ");
	Serial.print(msg.cell);
	Serial.print("Cmd: ");
	Serial.print(msg.command);
	Serial.print("Rsp: ");
	Serial.println(response);
}

void msgDebug(Message msg){
	byte buffer[msg.length()];
	msg.serialize((char*)buffer, msg.length());
	for (int i = 0; i < msg.length(); i++){
		Serial.write(buffer[i]);
	}
}