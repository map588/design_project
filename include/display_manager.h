#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define  LCD  LCD_2IN
#define hex_0  5
#define hex_1  2
#define hex_2  3
#define hex_3  4
#define buzzer 26

#include "definitions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "text_properties.h"
#include "pwm-tone.h"
#include "melodies.h"
#include <ctype.h>
#include <math.h>


static bool fired;
static uint16_t value;
static uint8_t action;
static uint8_t score_d;
static states state = LOADING;
static uint8_t idx;
static struct tonegenerator_t tone_gen;

bool select_state = 0;
bool game_state = 0;
bool load_state = 0;
bool key_state = 0;
bool restart_state = 0;




static uint16_t *s_buffer;



inline static void clearflags(){
      select_state = false;
      game_state = false;
      load_state = false;
      key_state = false;
      restart_state = false;
}

inline static void selction (){

    const int arr_pos[3] = {64, 152, 240};
    static uint8_t last_key = 0;
    uint8_t key = action;

    if(last_key > 0 && key == 0)
        last_key--;
    else if(last_key < 2 && key == 1)
        last_key++;
    else if(key == 3)
        last_key = 0;

    //  Paint_SelectImage((UBYTE *)s_buffer);

    if (!fired || !select_state){
        clearflags();
        select_state = true;
         // Paint_Clear(BLACK);
         // Paint_DrawString_EN(110, 14, "Which Wire?", &Font16, WHITE, BLACK);
         // Paint_DrawString_EN(44, 55, "(easy)", &Font12, WHITE, BLACK);
         // Paint_DrawString_EN(124, 55, "(medium)", &Font12, WHITE, BLACK);
         // Paint_DrawString_EN(224, 55, "(hard)", &Font12, WHITE, BLACK);

         // Paint_DrawLine(70, 80, 70, 200, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
         // Paint_DrawLine(158, 80, 158, 200, BLUE, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
         // Paint_DrawLine(246, 80, 246, 200, RED, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
         // Paint_DrawString_EN(arr_pos[0], 210, "^", &Font20, WHITE, BLACK);
         // LCD_2IN_Display((UBYTE *)s_buffer);
      }
    // // Paint_ClearWindows(0, 205, 319, 239, BLACK); // bottom bar
    int arrow_pos = arr_pos[last_key];


    // Paint_DrawString_EN(arrow_pos, 210, "^", &Font20, WHITE, BLACK);
    // LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
}

inline static void prompt_start(){
  printf("prompt_start\n");
  // Paint_SelectImage((UBYTE *)s_buffer);
  // Paint_Clear(BLACK);
  // Paint_DrawString_EN(35, 112, "PRESS ENTER TO CONTINUE", &Font16, GREEN, BLACK);
  // Paint_DrawRectangle(33, 110, 300, 130, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  // LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void countdown_to_start(){
  // Paint_SelectImage((UBYTE *)s_buffer);
  // Paint_Clear(BLACK);
  switch(idx){

    case 0:
      // Paint_DrawString_EN(154, 110, "3", &Font20, GREEN, BLACK);
      tone(&tone_gen, NOTE_DS3, 50);
      break;
    case 1:
      // Paint_DrawString_EN(154, 110, "2", &Font20, GREEN, BLACK);
      tone(&tone_gen, NOTE_DS3, 50);
      break;
    case 2:
      // Paint_DrawString_EN(154, 110, "1", &Font20, GREEN, BLACK);
      tone(&tone_gen, NOTE_DS3, 50);
      break;
    case 3:
      // Paint_DrawString_EN(45, 110, "DEFUSE THE BOMB!", &Font20, RED, BLACK);
      idx = 9;
      tone(&tone_gen, NOTE_C4, 50);
      break;
    default:
      break;
  }
  // LCD_2IN_Display((UBYTE *)s_buffer);
  return;
}

inline static void loading_bar(){
  uint16_t x1;
  uint16_t x2;
  uint16_t y1;
  uint16_t y2;

  y1 = 225;
  y2 = 235;
  // Paint_SelectImage((UBYTE *)s_buffer);

  if (!fired && !load_state){
        clearflags();
        load_state = true;
        // Paint_Clear(BLACK);
        // LCD_2IN_Display((UBYTE *)s_buffer);
  } 

  if (idx == 0){
      // Paint_ClearWindows(205, y1 - 2, 319, y2 + 2, BLACK);
      x1 = 220;
      x2 = 225;

    for(int i = 1; i <= 9; i++){
          x1 = 220 + 10 * (i);
          x2 = 220 + 10 * (i + 1) - 5;
          // Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
    }
    // LCD_2IN_Display((uint8_t *)s_buffer);
    // LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
  }
  else {
      // Paint_ClearWindows(205, 223, 215 + 10 * (idx + 1) + 3 , y2 + 2, BLACK);
  }
      
    if(idx >= 9){
        // LCD_2IN_DisplayWindows(205, y1 - 2, 319, y2 + 2, s_buffer);
        // Paint_ClearWindows(205, 223, 319, 239, BLACK);
    }

      // LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
}

inline static void populate_UI_elements(){
 
  // Paint_SelectImage((UBYTE *)s_buffer);
  // Paint_ClearWindows(0, 0, 319, 29, BLACK);   // top bar
  // Paint_ClearWindows(0, 220, 120, 239, BLACK); // approximately time window
  uint8_t round = score_d / 20;
  uint8_t n_round = 20 - (score_d % 20);

  char score_str[20]; 
  char round_str[20]; 
  char nextR_str[20]; 
  char  time_str[20];  

  sprintf(score_str, "SCORE: %u", score_d);
  sprintf(round_str, "ROUND: %u",  round);
  sprintf(nextR_str, "NEXT ROUND: %u", n_round);
  sprintf(time_str, "TIME: %u ms", value);

  
  // Paint_DrawString_EN(2,   2, score_str, &Font12,  GREEN, BLACK);
  // Paint_DrawString_EN(2,  16, nextR_str, &Font12, YELLOW, BLACK);
  // Paint_DrawString_EN(2, 226,  time_str, &Font12,   CYAN, BLACK);
  // Paint_DrawString_EN(260, 2, round_str, &Font12,    RED, BLACK);

  printf("%s\n", score_str);
  
  // LCD_2IN_DisplayWindows(0, 0, 30, 319, s_buffer);
  // LCD_2IN_DisplayWindows(224, 0, 239, 100, s_buffer);
}


inline static void write_prompt(){
  const char *prompt_str[3] = {"   TURN IT   ", "   YANK IT   ", "   WIRE IT   "};

  // Paint_SelectImage((UBYTE *)s_buffer);
  // Paint_ClearWindows(52, 100, 265, 140, BLACK); // rough prompt window

  // Paint_DrawString_EN(62, 110, prompt_str[action], &Font20, GRED, BLACK);
  // Paint_DrawRectangle(57, 105, 263, 135, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

  printf("%s\n", prompt_str[action]);

  // LCD_2IN_DisplayWindows(100, 52, 140, 268, s_buffer);
}


inline static void game_UI(){
   // Paint_SelectImage((UBYTE *)s_buffer);

   if (!fired){
      clearflags();
      game_state = true;
      // Paint_Clear(BLACK);
      write_prompt(action);
      populate_UI_elements(value, score_d);
      // LCD_2IN_Display((UBYTE *)s_buffer);
      fired = true;
    }

    tone(&tone_gen, NOTE_C3, value/20);
    loading_bar();
}


inline static void drive_hex(uint8_t hex){
  gpio_put(hex_0,  hex & 0x01);
  gpio_put(hex_1, (hex & 0x02) >> 1);
  gpio_put(hex_2, (hex & 0x04) >> 2);
  gpio_put(hex_3, (hex & 0x08) >> 3);
}

inline static void correct_disp(){
  // Paint_SelectImage((UBYTE *)s_buffer);
  // Paint_Clear(BLACK);
  // Paint_DrawString_EN(110, 100, "CORRECT", &Font20, GREEN, BLACK);
  // LCD_2IN_Display((UBYTE *)s_buffer);
  printf("CORRECT\n");
  melody(&tone_gen, POSITIVE, 1);
}

 inline static void incorrect_disp(){
  // Paint_SelectImage ((UBYTE *)s_buffer);
  // Paint_Clear(BLACK);
  // Paint_DrawString_EN (110, 100, "BOOM", &Font20, RED, BLACK);
  // LCD_2IN_Display((UBYTE *)s_buffer);
  printf("BOOM\n");
  melody(&tone_gen, NEGATIVE, 1);
}

inline static void play_again(){
  if (!fired)
  {
    clearflags();
    restart_state = true;
    // Paint_SelectImage((UBYTE *)s_buffer);
    // Paint_Clear(BLACK);
    // Paint_DrawString_EN(110, 100, "PLAY AGAIN?", &Font20, GREEN, BLACK);
    // LCD_2IN_Display((UBYTE *)s_buffer);
    printf("PLAY AGAIN?\n");
    fired = true;
  }
    loading_bar();
}


inline static void display_key(){
  static bool calculator_mode = false;
  static bool insert = false;
  static int  str_idx = 0;

  static char str_buffer [1024];


  static int calc_idx = 0;
  static int base = 10;

  long long op1;
  char *endptr = str_buffer;
  char operand;
  long long op2;
  long long result;
  

  if(!fired && !key_state){
    // Paint_Clear(BLACK);
    key_state = true;
  }

  if(str_idx >= 1024){
    printf("Buffer full, hit delete or numlock->insert to clear\n");
    return;
  }


  // Paint_SelectImage((uint8_t *) s_buffer);

  char character = (char)score_d;

    switch(character){
      case '\b':
        if ( str_idx > 0){
          str_buffer[str_idx - 1] = '\0';
          str_idx--;
        }
        break;
      case 0xAB:
        character = '<';
        goto put_char;
      case 0xBB:
        character = '>';
        goto put_char;
      case 'i':
        insert = !insert;
        break;
      case 'c':
        str_buffer[0] = '\0';
        str_idx = 0;
        break;
      case '<':
      case '>':
        break;
      case '\n':
      if(!calculator_mode){
        character = '\n';
        goto put_char;
      }
      else{
        for(int i = 1; i < str_idx; i++){
          if(str_buffer[str_idx - i] == '\n' || i == str_idx - 1){
              calc_idx = str_idx - i;
              break;
          }
        }
        char calc_buffer[512];
        for(int i = 0; i < (str_idx - calc_idx); i++){
          calc_buffer[i] = str_buffer[calc_idx + i];
        }
        op1 = strtoll(calc_buffer, &endptr, base);
        
        //Change of base
        if(*endptr == '\0' && calc_buffer[0] != '\0' && op1 > 0 && op1 <= 16){
          base = op1;
          printf("Base changed to %d\n", base);
        }
        else if(*endptr == '\0'){
          printf("Invalid base\n");
        }
        else{
        operand = *endptr;

        //accumulate partial results
        while(operand != '\0'){
          op2 = strtoll(endptr + 1 , &endptr, base);
          switch (operand){
            case '+':
              op1 = op1 + op2;
              break;
            case '-':
              op1 = op1 - op2;
              break;
            case '*':
              op1 = op1 * op2;
              break;
            case '/':
              op1 = (op2 == 0) ? 0 : op1 / op2;
              break;
            case '%':
              op1 = (op2 == 0) ? 0 : op1 % op2;
              break;
            case '!':
              op1 = !op2;
              break;
            case '&':
              op1 = op1 & op2;
              break;
            case '|':
              op1 = op1 | op2;
              break;
            case '^':
              op1 = pow(op1, op2);
              break;
            case '#':
              op1 = op1 ^ op2;
              break;
            case '<':
              op1 = op1 << op2;
              break;
            case '>':
              op1 = op1 >> op2;
              break;
            default:
              break;
          }
        operand = *endptr;
        }

        result = op1;
        str_buffer[str_idx] = ' ';
        str_buffer[str_idx + 1] = '=';
        str_buffer[str_idx + 2] = ' ';
        str_buffer[str_idx + 3] = '\0';
        str_idx += 3;
        char result_str[256];
        int result_idx = 0;

        sprintf(result_str, "%lld", result);
       
        while(result_str[result_idx] != '\0'){
          str_buffer[str_idx] = result_str[result_idx];
          str_idx++;
          result_idx++;
        }
        str_buffer[str_idx] = '\n';
        str_buffer[str_idx + 1] = '\0';
        str_idx++;
        
        endptr = str_buffer + str_idx;
        calc_buffer[0] = '\0';
        break;
      }
      }
      default:
  put_char:
        str_buffer[str_idx] = (char)character;
        str_buffer[str_idx + 1] = '\0';
        str_idx++;
        break;
    }

    printf("%c%c%c%c", 0x1B, 0x5B, 0x32, 0x4A); //This clears the serial terminal
    printf("%s    %u %u", str_buffer, str_idx,calc_idx);
    tone(&tone_gen, NOTE_A3, 100);

    if(str_idx >= 5){
      if(str_buffer[str_idx - 5] == '.' && str_buffer[str_idx - 4] == '.' && 
         str_buffer[str_idx - 3] == '.' && str_buffer[str_idx - 2] == '.' && 
         str_buffer[str_idx - 1] == '.'){
        calculator_mode = !calculator_mode;
        str_idx = 0;
        str_buffer[str_idx] = '\0';
        if(!calculator_mode){
        printf("Calculator mode disengaged.\n");
        melody(&tone_gen, CONFIRM, 1);
        }
        else{
        printf("Calculator mode engaged.\n");
        melody(&tone_gen, REJECT, 1);
        }
      }
    }
  
  
  
   // Paint_ClearWindows(0, 0, 320, 18, BLACK);
   // Paint_DrawString_EN(5, 5, str_buffer, key_text.font_size, key_text.color, key_text.background);
   // LCD_2IN_Display((uint8_t *)s_buffer);
}


// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 