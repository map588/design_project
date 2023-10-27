#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../Config/DEV_Config.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "pico/stdlib.h"
#include "pico/time.h"


enum states {INIT, SELECT, LOADING, GAME, CORRECT, INCORRECT, RESTART};
typedef enum {LEFT, RIGHT, ENTER} select;

    UWORD* init_disp(void);

    void selection_disp(select key);

    void loading_disp(float progress);

    void init_game_disp(void);

    void game_disp(int score, int stage, int time);
    
    void correct_disp(void);

    void incorrect_disp(void);

    void end_disp(int score);

    void restart_disp(void);

#endif 