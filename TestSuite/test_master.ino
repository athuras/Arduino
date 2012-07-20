#include <Wire.h>
#include <Message.h>
#include <avr/pgmspace.h>

prog_char unlockcode[] PROGMEM = {49, 49, 49, 49, 49, 49, 49, 49};
prog_char querycode[] PROGMEM =   {50, 50, 50, 50, 50, 50, 50, 50};
prog_char new_addresscode[] PROGMEM =   {51, 51, 51, 51, 51, 51, 51}; 
prog_char limitswitchcode[] PROGMEM = {52, 52, 52, 52, 52, 52, 52, 52};

PROGMEM const char *string_table[] = {unlockcode, querycode, new_addresscode, limitswitchcode};

const int COMMAND_LENGTH = 10;
void setup(){
  Serial.begin(9600);
  while (!Serial){;} // Apparently needed for Serial on Leonardo
  Wire.begin();
}

void loop(){
  if (Serial.available() > 0) {
  ;
  }

}
