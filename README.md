Arduino
=======

Arduino Project:
Terms used throughout this project:
	"Front End" - the typical software 'backend', the entity from whom instructions are given
	"Slave" - Microcontrollers connected directly to hardware elements, specificaly the locks, and sensors
	"Master" - Single Microcontroller responsible for interfacing between the Front-end and the slaves

	"Message" - A sequence of bytes of length COMMAND_LENGTH
			>> The first two bytes contain an address from 0-127 (note slaves cannot occupy certain addresses)
			>> The rest of the bytes contain both a 'cell' and an instruction 'string'

