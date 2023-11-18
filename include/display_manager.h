#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define hex_0  5
#define hex_1  2
#define hex_2  3
#define hex_3  4

#include "definitions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
//#include "../lib/GUI/GUI_Paint.h"



void core_one_interrupt_handler();

void core_one_main();

void display_exit();

// inline void selection_display(select key);

// inline void init_game_disp(void);

// inline void game_disp(int score, int stage, int time);

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 