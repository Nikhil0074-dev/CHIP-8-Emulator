#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "chip8.h"

#define SCALE          10          /* Each CHIP-8 pixel = 10×10 screen px */
#define WINDOW_TITLE   "CHIP-8 Emulator"

typedef struct Display Display;

Display *display_create(void);
void     display_destroy(Display *d);
void     display_render(Display *d, const uint8_t *pixels);
bool     display_poll_events(Display *d, Chip8 *c8);

#endif /* DISPLAY_H */
