#ifndef COLOUR_H
#define COLOUR_H

#include <stdint.h>

struct rgb_colour {
	uint8_t red, green, blue;
};

struct hsv_colour {
	uint8_t hue, saturation, value;
};

// State machine to make Colour library work asynchronously
enum colour_state {
	OFF,
	IDLE,
	WAIT,
	FADE_RGB,
	FADE_HSV
};

void colour_init(void);

// Called once every 8 ms
void tick(void);

void wait(uint16_t duration_in);

void callback_colour_finished(void);

// 
enum colour_state get_state(void);

void set_rgb(struct rgb_colour to);
void fade_rgb(struct rgb_colour to, uint16_t duration);


//To be implemented
void set_hsv(struct hsv_colour to);
void fade_hsv(struct hsv_colour to, uint16_t duration);

#endif /* COLOUR_H */
