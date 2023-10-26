#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../Config/DEV_Config.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"

#define IMAGE_SIZE LCD_2IN_HEIGHT * LCD_2IN_WIDTH  * 2


bool init_display(void);
void start_game_display(void);

#endif 