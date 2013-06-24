
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "Colour.h"


struct rgb_colour rgb_current = {0,0,0};
struct rgb_colour rgb_fade_to;
struct {int8_t red, green, blue} rgb_fade_step;

uint16_t duration;

enum colour_state state = OFF;

void hardware_rgb(void) {
    OCR1A = ~rgb_current.blue;
    OCR1B = ~rgb_current.green;
    OCR1C = ~rgb_current.red;        
}

void colour_init() {
	// Set LED pins to output
	DDRB |= (1 << PB7);
	DDRC |= (1 << PC6) | (1 << PC5);

	// 10 bit fast-pwm - inverse mode so we get the brightness of zero to LED off
  TCCR1A |= (1 << COM1A1) | (1 << COM1A0) |
            (1 << COM1B1) | (1 << COM1B0) |
            (1 << COM1C1) | (1 << COM1C0) |
            (1 << WGM11)  | (1 << WGM10); 

	// clock/1 prescaler
  //TCCR1B |= (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);

	// clock/8 prescaler
  TCCR1B |= (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

  // Overflow interrupt enabled for fading.
  TIMSK1 |= (1 << TOIE1);

  // Initial colour setting
  hardware_rgb();

  state = OFF;
}

ISR (TIMER1_OVF_vect) {
	static uint8_t microticks = 0;

	// Approx. every 10ms
	if(++microticks == 78) {
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
