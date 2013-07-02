#ifndef PROGRAM_H
#define PROGRAM_H

#include "ColourTime.h"

void program_init(void);


enum program_addr_space {
	ADDR_RAM = 0,
	ADDR_ROM = 1
};

/* Major opcodes (first nibble):
 * 0: halt
 * 1: set colour
 * 2: fade to colour
 * 3: wait
 * 8: goto
 */

enum program_opcode {
	OP_HALT = 0x00,
	OP_SET_RGB = 0x10,
	OP_SET_HSV = 0x11,
	OP_FADE_RGB = 0x20,
	OP_FADE_HSV = 0x21,
	OP_WAIT = 0x30,
	OP_GOTO = 0x80,
	OP_GOTO_ROM = 0x81,
	OP_GOTO_RAM = 0x82
};

void execute(uint8_t *code, unsigned int len);
void program_step(void);

#endif /* PROGRAM_H */