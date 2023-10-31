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
{ACTION_1 = 0x10000, ACTION_2 = 0x20000, ACTION_3 = 0x30000, ACTION_4 = 0x40000} actions;



void core_one_interrupt_handler(void);
void core_one_entry(void);

bool init_display(void);

inline uint32_t assemble_packet(states state, uint8_t index, actions action, uint8_t score, uint32_t data);

// inline void selection_display(select key);

// inline void init_game_disp(void);

// inline void game_disp(int score, int stage, int time);

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 