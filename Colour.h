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
	IDLE_RGB,
	FADE_RGB,
	IDLE_HSV,
	FADE_HSV
};

void colour_init(void);

// Call once every 10 ms
void tick(void);

// 
enum colour_state get_state(void);

void set_rgb(struct rgb_colour to);
void fade_rgb(struct rgb_colour to, uint16_t duration);


//To be implemented
void set_hsv(struct hsv_colour to);
void fade_hsv(struct hsv_colour to, uint16_t duration);

#endif /* COLOUR_H */
