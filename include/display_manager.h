#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../Config/DEV_Config.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/multicore.h"


typedef enum //Lookup for the state of the game
{LOADING, SELECT, GAME, CORRECT, INCORRECT, RESTART} states;

typedef enum //Lookup for the action of the game
{ACTION1 = 0x10000, ACTION2 = 0x20000, ACTION3 = 0x30000, ACTION4 = 0x40000} actions;

typedef enum  //Lookup for the stage of the game
{STAGE_1 = 0x10, STAGE_2 = 0x20, STAGE_3 = 0x30, STAGE_4 = 0x40, STAGE_5 = 0x50} stage;



void core_one_interrupt_handler(void);
void core_one_entry(void);

bool init_display(void);

// inline void selection_disp(select key);

// inline void loading_disp(float progress);

// inline void init_game_disp(void);

// inline void game_disp(int score, int stage, int time);

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 