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
#include "text_properties.h"
//#include "../lib/GUI/GUI_Paint.h"


bool select_state = false;
bool game_state   = false;
bool load_state   = false;
bool key_state    = false;


static uint16_t value = 1000;
static uint8_t action = 0;
static uint8_t score = 0;
static states state = LOADING;

static repeating_timer_t idx_timer;
static alarm_pool_t *core1_pool;
static uint8_t index = 0;

static uint16_t *s_buffer;

typedef void (*function_ptr)(void);
typedef struct{
  function_ptr func;
  bool repeating;
}function_holder;


 inline void clearflags(){
  select_state = false;
  game_state = false;
  load_state = false;
  key_state = false;}

 inline static void select_display (){

  const int arr_pos[3] = {64, 152, 240};
  static uint8_t last_key = 0;
 
  uint8_t key = action;
  if(last_key > 0 && key == 0)
    last_key--;
  else 
  if(last_key < 2 && key == 1)
    last_key++;

  Paint_SelectImage((UBYTE *)s_buffer);
  if (!select_state){
    clearflags();
    Paint_Clear(BLACK);
    select_state = true;
    Paint_DrawString_EN(110, 14, "Which Wire?", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(44, 55, "(easy)", &Font12, WHITE, BLACK);
    Paint_DrawString_EN(124, 55, "(medium)", &Font12, WHITE, BLACK);
    Paint_DrawString_EN(224, 55, "(hard)", &Font12, WHITE, BLACK);

    Paint_DrawLine(70, 80, 70, 200, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
    Paint_DrawLine(158, 80, 158, 200, BLUE, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
    Paint_DrawLine(246, 80, 246, 200, RED, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
    LCD_2IN_Display((UBYTE *)s_buffer);
   }
   Paint_ClearWindows(0, 205, 319, 239, BLACK); // bottom bar
   int arrow_pos = arr_pos[last_key];

  Paint_DrawString_EN(arrow_pos, 210, "^", &Font20, WHITE, BLACK);
  LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
}

inline static void countdown_bar(){
  uint16_t x1;
  uint16_t x2;
  uint16_t y1;
  uint16_t y2;

  y1 = 225;
  y2 = 235;

   if (load_state)
   {
     clearflags();
     Paint_Clear(BLACK);
     LCD_2IN_Clear(BLACK);
    }
    

   
  if (index == 0){
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(205, y1 - 2, 319, y2 + 2, WHITE);
    x1 = 220;
    x2 = 225;
    } 
    else {
    x1 = 220 + 10 * index;
    x2 = 220 + 10 * (index + 1) - 5;
    } 

    Paint_DrawRectangle(x1, y1, x2, y2, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
    LCD_2IN_DisplayWindows(y1 - 2, 205, y2 + 2, 319, s_buffer);

    if(index == 9){
      LCD_2IN_DisplayWindows(y1 - 2, 205, y2 + 2, 319, s_buffer);
      Paint_ClearWindows(205, y1 - 2, 319, y2 + 2, WHITE);
    }

      LCD_2IN_DisplayWindows(y1 - 2, 205, y2 + 2, 319, s_buffer);
      // LCD_2IN_Display((UBYTE *)s_buffer);
}


static void populate_UI_elements(){
    uint16_t countdown = value;
    uint8_t score = score;
 
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 0, 319, 29, WHITE);   // top bar
    Paint_ClearWindows(0, 220, 120, 239, WHITE); // approximately time window
    uint8_t round = score / 20;
    uint8_t n_round = 20 - (score % 20);
               
    char score_str[20]; 
    char round_str[20]; 
    char nextR_str[20]; 
    char  time_str[20];  

    sprintf(score_str, "SCORE: %u", score);
    sprintf(round_str, "ROUND: %u",  round);
    sprintf(nextR_str, "NEXT ROUND: %u", n_round);
    sprintf(time_str, "TIME: %u ms", countdown);

    // const text_properties UI_Text[4] = {
    //     {2, 2, GREEN, BLACK, &Font12, "SCORE :", 56},   // top left row 1
    //     {2, 16, YELLOW, BLACK, &Font12, "NEXT_R:", 56}, // top left row 2
    //     {260, 2, RED, BLACK, &Font12, "ROUND: ", 56},   // top right
    //     {2, 226, CYAN, BLACK, &Font12, "TIME: ", 48},   // bottom left
    // };

    Paint_DrawString_EN(2, 2, score_str, &Font12, GREEN, BLACK);

    Paint_DrawString_EN(2, 16, nextR_str, &Font12, YELLOW, BLACK);

    Paint_DrawString_EN(2, 226, time_str, &Font12, CYAN, BLACK);

    Paint_DrawString_EN(260, 2, round_str, &Font12, RED, BLACK);

    
    LCD_2IN_DisplayWindows(0, 0, 30, 319, s_buffer);
    LCD_2IN_DisplayWindows(224, 0, 239, 100, s_buffer);
}


static void write_prompt(){

        Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(52, 100, 265, 140, WHITE); // rough prompt window

    const char* prompt_str[3] = {"   TURN IT   ", "   YANK IT   ", "   WIRE IT   "}; 
    Paint_DrawString_EN(62, 110, prompt_str[action], &Font20, GRED, BLACK);

    Paint_DrawRectangle(57, 105, 263, 135, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

    LCD_2IN_DisplayWindows(100, 52, 140, 268, s_buffer);
}



 inline static void game_UI(){

  if(!game_state){
    clearflags();
    Paint_Clear(BLACK);
    game_state = true;
  }
  if(index == 0){
  write_prompt(action);
  populate_UI_elements(value, score); 
  }

  countdown_bar();
}

inline static void drive_hex(uint8_t hex)
{
  gpio_put(hex_0, hex & 0x01);
  gpio_put(hex_1, (hex & 0x02) >> 1);
  gpio_put(hex_2, (hex & 0x04) >> 2);
  gpio_put(hex_3, (hex & 0x08) >> 3);
}

inline static void correct_disp()
{
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 0, 319, 239, WHITE);
    Paint_DrawString_EN(110, 100, "CORRECT", &Font20, GREEN, BLACK);
    LCD_2IN_Display((UBYTE *)s_buffer);
}

 inline static void incorrect_disp(){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 239, WHITE);
  Paint_DrawString_EN (110, 100, "INCORRECT", &Font20, RED, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
  }


 inline static void displayPacket( uint16_t value, uint8_t score, uint8_t index, uint8_t action, uint8_t state){

  char packet_s[23];
  char action_s[5];
  char  state_s[5];

  const char *state_str[7] = {"LOAD", "SEL ", "GAME", "PASS", "FAIL", "RST ", "KEY "};
  const char *action_str[4] = {"TURN", "YANK", "WIRE", "NOP "};

  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 40, 150, 56, WHITE);

  sprintf(action_s, "%s", action_str[action]);
  sprintf(state_s, "%s", state_str[state]);
  sprintf(packet_s, "%u_%u #%u %s %s", value, score, index, action_s, state_s);
  Paint_DrawString_EN (0, 42, packet_s, &Font12, WHITE, BLACK);
  packet_s[0] = '\0';
  LCD_2IN_DisplayWindows(40, 0, 56, 150, s_buffer);
}

inline static void display_time_delta(uint64_t delta){
  char delta_s[23];
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(30, 55, 240, 78, WHITE);
  sprintf(delta_s, "Delta: %u ms", delta);
  Paint_DrawString_EN (30, 56, delta_s, &Font12, WHITE, BLACK);
  delta_s[0] = '\0';
  LCD_2IN_DisplayWindows(55, 30, 78, 240, s_buffer);
}

 inline static void display_key(){

  uint8_t character = score;

  static int  str_idx = 0;
  static char str_buffer[256];

  if(!key_state && !load_state){
    clearflags();
    Paint_Clear(BLACK);
    key_state = true;
  }
   else if(load_state){ //Load state doesn't need to be cleared, its already mostly black
    clearflags();
    key_state = true;
  }

  Paint_SelectImage((uint8_t *) s_buffer);
  if(character == 0x66){ //backspace
    str_buffer[str_idx] = '\0';
    if(str_idx > 0){str_idx--;}
  }
  else{
  str_buffer[str_idx] = (char)character;
  str_buffer[str_idx + 1] = '\0';
  str_idx++;
  }

  Paint_ClearWindows(0, 0, 320, 18, WHITE);
  Paint_DrawString_EN(5, 5, str_buffer, key_text.font_size, key_text.color, key_text.background);

  LCD_2IN_Display((uint8_t *)s_buffer);

}


// inline void selection_display(select key);

// inline void init_game_disp(void);

// inline void game_disp(int score, int stage, int time);

// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 