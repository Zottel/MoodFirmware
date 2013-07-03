#include "ColourTime.h"
#include "Program.h"

#include <stdbool.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

enum program_addr_space current_addrspace = ADDR_ROM;

// Space to permanently store instructions
#define ROM_SIZE 512
uint8_t rom[ROM_SIZE] EEMEM = {
	OP_FADE_RGB, 0xcc, 0x00, 0x00, 0x00, 0x40,
	OP_FADE_RGB, 0xee, 0x00, 0x00, 0x00, 0x20,
	OP_FADE_RGB, 0xcc, 0x00, 0x00, 0x00, 0x20,
	OP_FADE_RGB, 0xdd, 0x00, 0x00, 0x00, 0x40,
	OP_FADE_RGB, 0xcc, 0x00, 0x00, 0x00, 0x40,
	OP_WAIT, 0x04, 0x00,
	OP_GOTO, 0x00, 0x00
};
unsigned int rom_size = ROM_SIZE;


uint8_t *ram_base;
unsigned int ram_size = 0;


// Begin execution at start of rom
uint8_t *instruction = rom;


void program_init(void) {
	program_step();
}

void callback_colour_finished(void) {
	program_step();
}

uint8_t read_byte(void) {
	if(current_addrspace == ADDR_ROM) {
		if(instruction < rom || instruction >= (rom + ROM_SIZE)) {
			// TODO: error
			return 0x00;
		}

		return eeprom_read_byte(instruction++);
	} else {

		// TODO: sanity check
		if(instruction < ram_base || instruction >= (ram_base + ram_size)) {
			// TODO: error
			return 0x00;
		}

		return *(instruction++);
	}
}

void program_execute(uint8_t *program, unsigned int size) {
	// Being interrupted here would probably lead to inconsistencies
	cli();

	ram_base = program;
	ram_size = size;

	// Set execution to begin of buffer
	instruction = ram_base;

	current_addrspace = ADDR_RAM;

	// Execute to first blocking operation
	program_step();

	// Re-enable interrupts
	sei();
}

void program_step(void) {
	struct rgb_colour colour_rgb;
	struct hsv_colour colour_hsv;
	uint16_t duration = 0;

	switch((enum program_opcode) read_byte()) {
		case OP_HALT:
			instruction--;
			return;
		case OP_SET_RGB:
			colour_rgb.red = read_byte();
			colour_rgb.green = read_byte();
			colour_rgb.blue = read_byte();
			set_rgb(colour_rgb);
			// immediatly execute next step - since 
			program_step();
			break;

		case OP_SET_HSV:
			colour_hsv.hue = read_byte();
			colour_hsv.saturation = read_byte();
			colour_hsv.value = read_byte();
			set_rgb(colour_rgb);
			// immediatly execute next step - since 
			program_step();
			break;

		case OP_FADE_RGB:
			colour_rgb.red = read_byte();
			colour_rgb.green = read_byte();
			colour_rgb.blue = read_byte();
			duration |= (read_byte() << 8) | read_byte();
			fade_rgb(colour_rgb, duration);
			break;

		case OP_FADE_HSV:
			colour_hsv.hue = read_byte();
			colour_hsv.saturation = read_byte();
			colour_hsv.value = read_byte();
			duration |= (read_byte() << 8) | read_byte();
			//fade_hsv(colour_hsv, duration);
			break;

		case OP_WAIT:
			duration |= (read_byte() << 8) | read_byte();
			wait(duration);
			break;

		case OP_GOTO_ROM:
			current_addrspace = ADDR_ROM;

		case OP_GOTO:
			duration |= (read_byte() << 8) | read_byte();

			if(current_addrspace == ADDR_ROM) {
				instruction = rom + duration;
			} else {
				instruction = ram_base + duration;
			}
			program_step();
			break;
		default:
			break;
	}
}

