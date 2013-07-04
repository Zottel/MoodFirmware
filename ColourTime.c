
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "ColourTime.h"

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

// Currently used colour
struct rgb_colour rgb_current = {0,0,0};
struct hsv_colour hsv_current = {0,0,0};

// Fading parameters
struct rgb_colour rgb_fade_to, rgb_fade_from;
struct hsv_colour hsv_fade_to, hsv_fade_from = {0,0,0};
uint16_t duration, duration_done;



// State machine
enum colour_state state = OFF;

// Translate rgb values to logarithmic 16bit scale - and invert
void hardware_rgb(void) {
    OCR1A = ~(pgm_read_word(&pwmtable_16[rgb_current.blue]));
    OCR1B = ~(pgm_read_word(&pwmtable_16[rgb_current.green]));
    OCR1C = ~(pgm_read_word(&pwmtable_16[rgb_current.red]));
}

// Inverted pwm - without logarithm
/*void hardware_rgb(void) {
    OCR1A = ~(rgb_current.blue << 8);
    OCR1B = ~(rgb_current.green << 8);
    OCR1C = ~(rgb_current.red << 8);
}*/


// Taken from http://www.mikrocontroller.net/topic/54561 - mostly
void hardware_hsv() {
	struct rgb_colour res;

	uint8_t i, f;
	uint16_t p, q, t;

	if( hsv_current.saturation == 0 ) 
 	{	res.red = res.green = res.blue = hsv_current.value;
	}
	else
	{	i=hsv_current.hue/43;
		f=hsv_current.hue%43;
		p = (hsv_current.value * (255 - hsv_current.saturation))/256;
		q = (hsv_current.value * ((10710 - (hsv_current.saturation * f))/42))/256;
		t = (hsv_current.value * ((10710 - (hsv_current.saturation * (42 - f)))/42))/256;

		switch( i )
		{	case 0:
				res.red = hsv_current.value; res.green = t; res.blue = p; break;
			case 1:
				res.red = q; res.green = hsv_current.value; res.blue = p; break;
			case 2:
				res.red = p; res.green = hsv_current.value; res.blue = t; break;
			case 3:
				res.red = p; res.green = q; res.blue = hsv_current.value; break;			
			case 4:
				res.red = t; res.green = p; res.blue = hsv_current.value; break;				
			case 5:
	 			res.red = hsv_current.value; res.green = p; res.blue = q; break;
		}
	}

	rgb_current = res;

	hardware_rgb();
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

  // Overflow at 0xffff
  ICR1 = 0xffff;

	// clock/1 prescaler
  TCCR1B |= (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);

	// clock/8 prescaler
  //TCCR1B |= (0 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);

  // Overflow interrupt enabled for fading.
  TIMSK1 |= (1 << TOIE1);

  // Initial colour setting
  hardware_rgb();

  if(state == OFF)
	  state = IDLE;
}

ISR (TIMER1_OVF_vect) {
	tick();
}

void __attribute__ ((weak)) callback_colour_finished(void) {}

// Call once every 8 ms - circa
void tick() {
	switch(state) {
		case OFF:
		case IDLE:
			return;

		case WAIT:
			duration_done += 4;
			if(duration_done > duration)
			{
				state = IDLE;
				callback_colour_finished();
			}
			break;


		case FADE_RGB:
			duration_done += 4;
			if(duration_done > duration)
			{
				// Get rid of rounding errors
				rgb_current = rgb_fade_to;

				state = IDLE;

				callback_colour_finished();
			} else {
				rgb_current.red = (int32_t) rgb_fade_from.red +
				                            ((((int32_t) rgb_fade_to.red -
				                              (int32_t) rgb_fade_from.red)
				                              * (int32_t) duration_done)
				                            / (int32_t) duration);

				rgb_current.green = (int32_t) rgb_fade_from.green +
				                              ((((int32_t) rgb_fade_to.green -
				                                (int32_t) rgb_fade_from.green)
				                                * (int32_t) duration_done)
				                              / (int32_t) duration);

				rgb_current.blue = (int32_t) rgb_fade_from.blue +
				                             ((((int32_t) rgb_fade_to.blue -
				                               (int32_t) rgb_fade_from.blue)
				                               * (int32_t) duration_done)
				                             / (int32_t) duration);

			}

			hardware_rgb();
			break;

		case FADE_HSV:
			duration_done += 4;
			if(duration_done > duration)
			{
				// Get rid of rounding errors
				hsv_current = hsv_fade_to;

				state = IDLE;

				callback_colour_finished();
			} else {
				// TODO: Hue is a circle - have it wrap around properly?
				// (Overflow is a good thing here)
				hsv_current.hue = (int32_t) hsv_fade_from.hue +
				                            ((((int32_t) hsv_fade_to.hue -
				                              (int32_t) hsv_fade_from.hue)
				                              * (int32_t) duration_done)
				                            / (int32_t) duration);

				hsv_current.saturation = (int32_t) hsv_fade_from.saturation +
				                              ((((int32_t) hsv_fade_to.saturation -
				                                (int32_t) hsv_fade_from.saturation)
				                                * (int32_t) duration_done)
				                              / (int32_t) duration);

				hsv_current.value = (int32_t) hsv_fade_from.value +
				                             ((((int32_t) hsv_fade_to.value -
				                               (int32_t) hsv_fade_from.value)
				                               * (int32_t) duration_done)
				                             / (int32_t) duration);

			}

			hardware_hsv();


		default:
			break;
	}
}

enum colour_state get_state() {
	return state;
}

void wait(uint16_t duration_in) {
	// Prevent duration overflow
	if(duration_in > (0xffff - 8))
		duration_in = 0xffff - 8;

	duration = duration_in;
	duration_done = 0;

	state = WAIT;
}

void set_rgb(struct rgb_colour to) {
	state = IDLE;

	rgb_current = to;

	hardware_rgb();
}

void fade_rgb(struct rgb_colour to, uint16_t duration_in) {
	state = FADE_RGB;

	rgb_fade_from = rgb_current;

	rgb_fade_to = to;

	// Prevent duration overflow
	if(duration_in > (0xffff - 8))
		duration_in = 0xffff - 8;

	duration = duration_in;
	duration_done = 0;
}

//To be implemented
void set_hsv(struct hsv_colour to) {
	state = IDLE;
	hsv_current = to;
	hardware_hsv();
}
void fade_hsv(struct hsv_colour to, uint16_t duration_in) {
	state = FADE_HSV;

	// TODO: This is NOT sync'ed with rgb
	hsv_fade_from = hsv_current;
	hsv_fade_to = to;

	// Prevent duration overflow
	if(duration_in > (0xffff - 8))
		duration_in = 0xffff - 8;

	duration = duration_in;
	duration_done = 0;
}
