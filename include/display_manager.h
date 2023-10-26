#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../Config/DEV_Config.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#define IMAGE_SIZE LCD_2IN_HEIGHT * LCD_2IN_WIDTH  * 2

enum select { LEFT, RIGHT };
enum state {INIT, SELECT, LOADING, GAME, CORRECT, INCORRECT, RESTART};


bool init_disp(void);

void selection_disp(enum select selection);

void loading_disp(float progress);

void init_game_disp(void);

void game_disp(int score, int stage, int time);
 
void correct_disp(void);

void incorrect_disp(void);

void end_disp(int score);

void restart_disp(void);

#endif 