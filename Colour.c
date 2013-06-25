
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "Colour.h"

// Taken from http://www.mikrocontroller.net/articles/LED-Fading
const uint16_t pwmtable_16[256] PROGMEM =
{
    0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
    3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 7,
    7, 7, 8, 8, 8, 9, 9, 10, 10, 10, 11, 11, 12, 12, 13, 13, 14, 15,
    15, 16, 17, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    31, 32, 33, 35, 36, 38, 40, 41, 43, 45, 47, 49, 52, 54, 56, 59,
    61, 64, 67, 70, 73, 76, 79, 83, 87, 91, 95, 99, 103, 108, 112,
    117, 123, 128, 134, 140, 146, 152, 159, 166, 173, 181, 189, 197,
    206, 215, 225, 235, 245, 256, 267, 279, 292, 304, 318, 332, 347,
    362, 378, 395, 412, 431, 450, 470, 490, 512, 535, 558, 583, 609,
    636, 664, 693, 724, 756, 790, 825, 861, 899, 939, 981, 1024, 1069,
    1117, 1166, 1218, 1272, 1328, 1387, 1448, 1512, 1579, 1649, 1722,
    1798, 1878, 1961, 2048, 2139, 2233, 2332, 2435, 2543, 2656, 2773,
    2896, 3025, 3158, 3298, 3444, 3597, 3756, 3922, 4096, 4277, 4467,
    4664, 4871, 5087, 5312, 5547, 5793, 6049, 6317, 6596, 6889, 7194,
    7512, 7845, 8192, 8555, 8933, 9329, 9742, 10173, 10624, 11094,
    11585, 12098, 12634, 13193, 13777, 14387, 15024, 15689, 16384,
    17109, 17867, 18658, 19484, 20346, 21247, 22188, 23170, 24196,
    25267, 26386, 27554, 28774, 30048, 31378, 32768, 34218, 35733,
    37315, 38967, 40693, 42494, 44376, 46340, 48392, 50534, 52772,
    55108, 57548, 60096, 62757, 65535
};

struct rgb_colour rgb_current = {0,0,0};
struct rgb_colour rgb_fade_to;
struct {int8_t red, green, blue} rgb_fade_step;

uint16_t duration;

enum colour_state state = OFF;

void hardware_rgb(void) {
    OCR1A = ~(pgm_read_word(&pwmtable_16[rgb_current.blue]));
    OCR1B = ~(pgm_read_word(&pwmtable_16[rgb_current.green]));
    OCR1C = ~(pgm_read_word(&pwmtable_16[rgb_current.red]));
}

void colour_init() {
	// Set LED pins to output
	DDRB |= (1 << PB7);
	DDRC |= (1 << PC6) | (1 << PC5);

	// 16bit pwm - inverse mode so we get the brightness of zero to LED off
  TCCR1A |= (1 << COM1A1) | (1 << COM1A0) |
            (1 << COM1B1) | (1 << COM1B0) |
            (1 << COM1C1) | (1 << COM1C0) |
            (1 << WGM11)  | (0 << WGM10); 

  ICR1 = 0xffff;

	// clock/1 prescaler
  TCCR1B |= (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);

	// clock/8 prescaler
  //TCCR1B |= (0 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

  // Overflow interrupt enabled for fading.
  TIMSK1 |= (1 << TOIE1);

  // Initial colour setting
  hardware_rgb();

  state = OFF;
}

ISR (TIMER1_OVF_vect) {
	static uint8_t microticks = 0;

	// Approx. every 8ms
	if(++microticks == 2) {
		microticks = 0;
		tick();
	}
}

// Call once every 10 ms - circa
void tick() {
	bool modified = false;
	switch(state) {
		case OFF:
		case IDLE_RGB:
		case IDLE_HSV:
			return;

		case FADE_RGB:
			if(abs(rgb_fade_to.red - rgb_current.red) > abs(rgb_fade_step.red))
			{
				rgb_current.red += rgb_fade_step.red;
				modified = true;
			}

			if(abs(rgb_fade_to.green - rgb_current.green) > abs(rgb_fade_step.green))
			{
				rgb_current.green += rgb_fade_step.green;
				modified = true;
			}

			if(abs(rgb_fade_to.blue - rgb_current.blue) > abs(rgb_fade_step.blue))
			{
				rgb_current.blue += rgb_fade_step.blue;
				modified = true;
			}

			if(!modified) {
				state = IDLE_RGB;
				rgb_current = rgb_fade_to;
			}

			hardware_rgb();
			break;

		default:
			break;
	}
}

enum colour_state get_state() {
	return state;
}

void set_rgb(struct rgb_colour to) {
	rgb_current = to;

	hardware_rgb();

	state = IDLE_RGB;
}

void fade_rgb(struct rgb_colour to, uint16_t duration) {
	rgb_fade_to = to;

	rgb_fade_step.red = (to.red - rgb_current.red) / (duration / 10);
	rgb_fade_step.green = (to.green - rgb_current.green) / (duration / 10);
	rgb_fade_step.blue = (to.blue - rgb_current.blue) / (duration / 10);

	state = FADE_RGB;
}

//To be implemented
void set_hsv(struct hsv_colour to);
void fade_hsv(struct hsv_colour to, uint16_t duration);