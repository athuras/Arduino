// This is the slave microcontroller sketch

#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>

//set up commands into flash memory
prog_char unlockcode[] PROGMEM =      {49, 49, 49, 49, 49, 49, 49, 49};   // "String 0" etc are strings to store - change to suit.
prog_char querycode[] PROGMEM =       {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM = {51, 51, 51, 51, 51, 51, 51}; 
prog_char limitswitchcode[] PROGMEM = {52, 52, 52, 52, 52, 52, 52, 52};
prog_char CELL_TYPES[] PROGMEM =      {'A', 'B', 'C', 'C', 'B', 'A'}; // the length of this array relates to how many cells there are.

// Pins are arbitrary, and should be changed depending on the requirements.

const int CELL_COUNT = 0;
const int CONTROL_SIZE = 4 + 1; // the last '1' is for the limit switch
const int DEC_OUT = 10;
const int MUX_IN = 5; // must be analog in
const int muxSelectPins[CONTROL_SIZE] = {1,2,3,4,5}; // the 5th pin is to toggle limit switch
const int decodeControlPins[CONTROL_SIZE - 1] = {1,2,3,4};

const int PULSE_DELAY = 100; // in ms

PROGMEM const char *string_table[] = 	   // change "string_table" name to suit
{   
  unlockcode, querycode, new_addresscode, limitswitchcode
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
  for ( int i = 0; i < CONTROL_SIZE - 2; i++){ // note the decoder has one-fewer control pin
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
        reply( query( (int)received_command.cell) ); // returns status of cell opened
    } else if (memcmp(received_command.command, string_table[1], COMMAND_LENGTH)){ // queried by master
        
      reply( query(received_command.cell) );
    } else if (memcmp(received_command.command, string_table[2], COMMAND_LENGTH-1)){  // this state shouldn't happen 
    } else if (memcmp(received_command.command, string_table[3], COMMAND_LENGTH)){  //query limit switch
      // query limit switch status.
      reply( query( (int)received_command.cell + pow(2,CONTROL_SIZE-1) )); // Toggles the last MUX port to trigger limit switch.
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
  pinMode(pin,OUTPUT);
  digitalWrite(pin, HIGH);
  delay(PULSE_DELAY);
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
    pinMode(DEC_OUT,OUTPUT);
    pulse(DEC_OUT);
  }
}

Message query(byte cell){
 int reading = LOW;
  if (cell == 0){
    int proxy_cell_count[] = {CELL_COUNT, 0,0,0,0,0,0,0};
    return Message((char) current_address, 0, proxy_cell_count);
     // return the number of consecutaive cells to master. master should then sequentially query each cell.
  } else {
    char type = CELL_TYPES[cell - 1];
    muxSelect( (int) cell );
    pinMode(MUX_IN, INPUT);
    reading = digitalRead(MUX_IN);
    int body[] = {type, reading, 0,0,0,0,0,0}; 
    return Message((char) current_address,(int) cell, body);
  }
}

void reply(Message msg){
  int length = 10; //this is dummied
  char writeBuffer[length];
  msg.serialize(writeBuffer, length);
  Wire.write((byte*)writeBuffer, length);
  return;
}
