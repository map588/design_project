
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"


// PIO program for scanning the keyboard matrix
void keyboard_init(char *key, bool *key_press);
void get_code();

#endif