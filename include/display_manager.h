#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../Config/DEV_Config.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/multicore.h"

typedef struct
{
  uint16_t x1;
  uint16_t y1;
  uint16_t y2;
  uint16_t width;
} bar_properties;

const bar_properties load_properties = {
220, //start x
225, //start y
235, //end y
 10, //width
     //end x is start x + width * offset
};

typedef struct 
{
  uint16_t x;
  uint16_t y;
  uint16_t color;
  uint16_t background;
  sFONT *font_size;
  const char *text;
  uint8_t text_length;
} text_properties;

enum text {SCORE, NEXT, ROUND, TIME};
const text_properties UI_Text[4] = {
  {  2,    2, GREEN , BLACK, &Font12, "SCORE :", 56},  //top left row 1
  {  2,   16, YELLOW, BLACK, &Font12, "NEXT_R:", 56},  //top left row 2
  {260,    2, RED   , BLACK, &Font12, "ROUND :", 56},  //top right
  {  2,  226, CYAN  , BLACK, &Font12, "TIME  :", 56},  //bottom left
};

enum promt {ARM, REWIRE, YANK};
const text_properties UI_Prompt[3] = {
  {  62,  110, GRED  , BLACK, &Font20, "   ARM IT    ", 196},  //bottom left
  {  62,  110, GRED  , BLACK, &Font20, "  REWIRE IT  ", 196},  //bottom left
  {  62,  110, GRED  , BLACK, &Font20, "   YANK IT   ", 196},  //bottom left
};




typedef enum //Lookup for the state of the game
{LOADING, SELECT, GAME, CORRECT, INCORRECT, RESTART} states;

typedef enum //Lookup for the action of the game
{ACTION_1 = 0x10000, ACTION_2 = 0x20000, ACTION_3 = 0x30000, ACTION_4 = 0x40000} actions;


void core_one_interrupt_handler(void);
void core_one_entry(void);

bool init_display(void);

inline uint32_t assemble_packet(states state, uint8_t index, actions action, uint8_t score, uint16_t data);

// inline void selection_display(select key);

// inline void init_game_disp(void);

// inline void game_disp(int score, int stage, int time);

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 