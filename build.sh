#!/bin/bash

if make
then
	sudo dfu-programmer atmega32u2 erase
	sudo dfu-programmer atmega32u2 flash-eeprom Moodlight.eep 
	sudo dfu-programmer atmega32u2 flash Moodlight.hex
fi
