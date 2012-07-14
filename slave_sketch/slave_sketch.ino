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
prog_char querycode[] PROGMEM = {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =  {51, 51, 51, 51, 51, 51, 51}; 
// Pins are arbitrary, and should be changed depending on the requirements.
const int CONTROL_SIZE = 4;
const int DEC_OUT = 10;
const int MUX_IN = 5;
const int muxSelectPins[CONTROL_SIZE] = {1,2,3,4};
const int decodeControlPins[CONTROL_SIZE] = {6,7,8,9};

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
  pinMode(MUX_IN, INPUT);
  pinMode(DEC_OUT, OUTPUT);
  for ( int i = 0; i < CONTROL_SIZE - 1; i++){
    pinMode(muxSelectPins[i], OUTPUT);
  }
  for ( int i = 0; i < CONTROL_SIZE -1; i++){
    pinMode(decodeControlPins[i], OUTPUT);
  }
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

// One-based. i.e. muxSelect( 1 ) selects the first (zeroith) mux in.
void muxSelect(int id){ 
 int aux = id - 1;
 for ( int k = CONTROL_SIZE - 1; k >= 0; k--){
    int a = aux % (int) pow(2,k);
    if ( a == aux ){
      digitalWrite(muxSelectPins[k], LOW);
    }
    else {
      digitalWrite(muxSelectPins[k], HIGH);
      aux = aux - a;
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
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
  return;
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
      decSelect( (int) cell );
      pulse(DEC_OUT);
    }
    return;
}

void query(byte cell){
  if (cell == 0){
     // return the number of consecutaive cells to master. master should then sequentially query each cell.
  } else {
    muxSelect( (int) cell );
    // read value of MUX_IN
  }
}
