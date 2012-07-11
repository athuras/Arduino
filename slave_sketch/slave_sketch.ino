// This is the slave microcontroller sketch

/*
Needs to know:
  List of Cells
  I2C Address
  Connection/Pin Schema for logic
  ADC calibration data
Needs to perform:
  Respond to Query Command
    ADC sensor data
  Respond to Unlock Command
*/

#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>

//set up commands into flash memory
prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51} ; 

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{   
  unlockcode, querycode, new_addresscode
};

const byte DEFAULT_ADDRESS = 99;
byte current_address = 99;
const int COMMAND_LENGTH = 12;
Message received_command = Message();

void setup(){
  Wire.begin(DEFAULT_ADDRESS);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop(){

}

void receiveEvent(int value){
  char buffer[COMMAND_LENGTH];
  int cnt = 0;
  received_command.empty();
  while (0 < Wire.available() && cnt < COMMAND_LENGTH){
     buffer[cnt] = Wire.read();
     cnt++;
  }
  received_command.deserialize(buffer, cnt);
  if (current_address == DEFAULT_ADDRESS){
      //compares the first 7 btyes of the received command with new_address command
      //if they match, then the 8th btyte [COMMAND_LENGTH-1] holds the new address to be assigned
      if (memcmp(received_command.command, string_table[2], COMMAND_LENGTH - 1)){ 
         resetAddress(received_command.command[COMMAND_LENGTH-1]); 
      }
  } else {
    if (memcmp(received_command.command, string_table[0], COMMAND_LENGTH)){ // unlock code sent
        unlock(received_command.cell);
    } else if (memcmp(received_command.command, string_table[1], COMMAND_LENGTH)){ // queried by master
        query(received_command.cell);
    } else if (memcmp(received_command.command, string_table[2], COMMAND_LENGTH-1)){  // this state shouldn't happen 
    }
  }
}

void resetAddress(byte address){
  current_address = address;
  Wire.begin(address);
  return;
}

void unlock(byte cell){
    if (cell == 0){
       //this is an error state 
    } else {
      //unlock cell
    }
}

void query(byte cell){
  if (cell == 0){
     //query the state of the column 
  } else {
     //query the state of the specified cell
  }
}
