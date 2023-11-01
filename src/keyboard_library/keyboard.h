
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"


// PIO program for scanning the keyboard matrix
void get_code();
int read_keyboard();
