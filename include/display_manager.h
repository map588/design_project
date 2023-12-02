#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define LCD LCD_2IN
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
#include "melodies.h"
#include "pwm-tone.h"
#include <ctype.h>

static bool fired;
static bool enabled;
static uint16_t value;
static uint8_t action;
static uint8_t score_d;
static states state = LOADING;
static uint8_t index;
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

inline static void enclosure(){
  Paint_SelectImage((UBYTE *)s_buffer);

  //clear area for enclosure
  Paint_ClearWindows(143, 35, 283, 202, BLACK);
  //draw the big rectangle for the enclosure
  Paint_DrawRectangle(146, 38, 280, 198, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the rectangle for the "display"
  Paint_DrawRectangle(160, 62, 266, 108, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the rectangle for the keypad
  Paint_DrawRectangle(177, 117, 248, 187, GRAY, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the rectangle for the hex display
  Paint_DrawRectangle(151, 176, 163, 193, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  
  //partially update the display for the enclosure
  //TODO: partial update
    LCD_2IN_DisplayWindows(35, 143, 202, 283, s_buffer);
}

//TODO: include ways to change color for these functions
inline static void yank_draw(uint8_t base_color){

  Paint_SelectImage((UBYTE *)s_buffer);

  //clear the area for the yank
  Paint_ClearWindows(105, 151, 145, 174, BLACK);

  //draw the 3 wires for the yank
  if(base_color == 1) //action is being prompted
  {
  Paint_DrawRectangle(108, 155, 145, 159, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 160, 145, 164, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 165, 145, 169, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }
  else if(base_color == 2) //action was performed correctly
  {
  Paint_DrawRectangle(108, 155, 145, 159, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 160, 145, 164, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 165, 145, 169, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }
  else //default colors
  {
  Paint_DrawRectangle(108, 155, 145, 159, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 160, 145, 164, BLUE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(108, 165, 145, 169, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }

  //TODO: partial update for yank
  LCD_2IN_DisplayWindows(151, 105, 174, 148, s_buffer);

}

inline static void turn_draw(uint8_t base_color){

  Paint_SelectImage((UBYTE *)s_buffer);

  //clear the area for the turn
  Paint_ClearWindows(281, 45, 318, 82, BLACK);

  if(base_color == 1)//action is prompted
  {
  //draw the base rectangle for the key
  Paint_DrawRectangle(281, 58, 307, 69, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the first circle for the key
  Paint_DrawCircle(307, 63, 10, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the inner circle for the key
  Paint_DrawCircle(307, 63, 4, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }
  else if(base_color == 2)//action was input correctly
  {
  //draw the base rectangle for the key
  Paint_DrawRectangle(281, 58, 307, 69, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the first circle for the key
  Paint_DrawCircle(307, 63, 10, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the inner circle for the key
  Paint_DrawCircle(307, 63, 4, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }
  else //base case, default colors
  {
  //draw the base rectangle for the key
  Paint_DrawRectangle(281, 58, 307, 69, YELLOW, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the first circle for the key
  Paint_DrawCircle(307, 63, 10, YELLOW, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the inner circle for the key
  Paint_DrawCircle(307, 63, 4, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  }

  //TODO: partial update for turn
  LCD_2IN_DisplayWindows(45, 278, 82, 319, s_buffer);

}

inline static void wire_draw(uint8_t base_color){

  Paint_SelectImage((UBYTE *)s_buffer);

  //clear the area for the wire
  Paint_ClearWindows(151, 24, 187, 56, BLACK);

  if(base_color == 1) //action is prompted
  {
  //draw a circle for the wire protruding from the top of the enclosure
  Paint_DrawCircle(168, 38, 8, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw a rectangle to cover the bottom half of the circle
  Paint_DrawRectangle(151, 38, 187, 56, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw 2 circles for the terminals
  Paint_DrawCircle(160, 47, 6, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw a line from where the circle meets the top to the middle of the left terminal
  Paint_DrawLine(160, 38, 160, 47, RED, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  }
  else if(base_color == 2) //action input successfully by player
  {
  Paint_DrawCircle(168, 38, 8, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(151, 38, 187, 56, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawLine(160, 38, 160, 47, GREEN, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  }
  else //default colors
  {
  Paint_DrawCircle(168, 38, 8, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(151, 38, 187, 56, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, BLUE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawLine(160, 38, 160, 47, WHITE, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  }

  //TODO: partial update for wire
  LCD_2IN_DisplayWindows(24, 151, 56, 187, s_buffer);

}


inline static void explosion_draw(int x_cen, int y_cen, double radfactor){
  Paint_SelectImage((UBYTE *)s_buffer); //select image

  //create layered circles
  //order: red->orange->yellow->white
  Paint_DrawCircle(x_cen, y_cen, 48*radfactor, RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(x_cen, y_cen, 32*radfactor, YELLOW, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(x_cen, y_cen, 16*radfactor, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);

  //need to partially update ONLY the area around the circle
  //take x_cen and y_cen, subtract both by 48*radfactor to get starting coords
  //add both by 48*radfactor to get end coords
  int adjustedrad = 48*radfactor;
  LCD_2IN_DisplayWindows(y_cen-adjustedrad, x_cen-adjustedrad, y_cen+adjustedrad, x_cen+adjustedrad, s_buffer);

}

inline static void selction (){

    const int arr_pos[3] = {64, 152, 240};
    static uint8_t last_key = 0;
    uint8_t key = action;

    if(last_key > 0 && key == 0)
        last_key--;
    else if(last_key < 2 && key == 1)
        last_key++;
    

    Paint_SelectImage((UBYTE *)s_buffer);

    if (!fired || !select_state){
        clearflags();
        select_state = true;
        Paint_Clear(BLACK);
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

inline static void prompt_start(){
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  Paint_DrawString_EN(35, 112, "PRESS ENTER TO CONTINUE", &Font16, GREEN, BLACK);
  Paint_DrawRectangle(33, 110, 300, 130, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void countdown_to_start(){    
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);

  //draw the bomb here, then do the switch case
  //move the countdown to the side, bomb on the right
  
  //call bomb function
  enclosure();
  //call turn function
  turn_draw(0);
  //call yank function
  yank_draw(0);
  //call wire function
  wire_draw(0);


  switch(index){ 
    case 0:
      Paint_DrawString_EN(75, 83, "3", &Font20, GREEN, BLACK);
      break;
    case 1:
      Paint_DrawString_EN(75, 83, "2", &Font20, GREEN, BLACK);
      break;
    case 2:
      Paint_DrawString_EN(75, 83, "1", &Font20, GREEN, BLACK);
      break;
    case 3:
      Paint_DrawString_EN(12, 83, "DEFUSE THE BOMB!", &Font20, RED, BLACK);
      index = 9;
      break;
    default:
      break;
  }
  LCD_2IN_Display((UBYTE *)s_buffer);
  return;
}


inline static void loading_bar(){
  uint16_t x1;
  uint16_t x2;
  uint16_t y1;
  uint16_t y2;

  y1 = 225;
  y2 = 235;
  Paint_SelectImage((UBYTE *)s_buffer);

  if (!fired && !load_state){
        clearflags();
        load_state = true;
        Paint_Clear(BLACK);
        LCD_2IN_Display((UBYTE *)s_buffer);
  } 

  if (index == 0){
      Paint_ClearWindows(205, y1 - 2, 319, y2 + 2, BLACK);
      x1 = 220;
      x2 = 225;

    for(int i = 1; i <= 9; i++){
          x1 = 220 + 10 * (i);
          x2 = 220 + 10 * (i + 1) - 5;
          Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
    }
    //LCD_2IN_Display((uint8_t *)s_buffer);
    LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
  }
  else {
      Paint_ClearWindows(205, 223, 215 + 10 * (index + 1) + 3 , y2 + 2, BLACK);
  }
      
    if(index >= 9){
        LCD_2IN_DisplayWindows(205, y1 - 2, 319, y2 + 2, s_buffer);
        Paint_ClearWindows(205, 223, 319, 239, BLACK);
    }

      LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
}

inline static void populate_UI_elements(){
 
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 29, BLACK);   // top bar
  Paint_ClearWindows(0, 220, 120, 239, BLACK); // approximately time window
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

  
  Paint_DrawString_EN(2,   2, score_str, &Font12,  GREEN, BLACK);
  Paint_DrawString_EN(2,  16, nextR_str, &Font12, YELLOW, BLACK);
  Paint_DrawString_EN(2, 226,  time_str, &Font12,   CYAN, BLACK);
  Paint_DrawString_EN(260, 2, round_str, &Font12,    RED, BLACK);

  
  LCD_2IN_DisplayWindows(0, 0, 30, 319, s_buffer);
  LCD_2IN_DisplayWindows(224, 0, 239, 100, s_buffer);
}


inline static void write_prompt(){     //BEING CHANGED
  const char *prompt_str[3] = {"TURN IT", "YANK IT", "WIRE IT"};

  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(6, 59, 145, 111, BLACK); // rough prompt window

  //draw the bomb graphic on screen
  enclosure();

  //ensure correct portion of graphic is highlighted red
  switch(action) {
    case 0:
      turn_draw(1);
      yank_draw(0);
      wire_draw(0);
    break;
    case 1:
      turn_draw(0);
      yank_draw(1);
      wire_draw(0);
    break;
    case 2:
      turn_draw(0);
      yank_draw(0);
      wire_draw(1);
    break;
    default:
    break;
  }

  //writes in the action to be done and the rectangle encompassing the string
  Paint_DrawString_EN(20, 76, prompt_str[action], &Font20, GRED, BLACK);
  Paint_DrawRectangle(17, 73, 140, 101, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

  //dotted lines from corners of graphic display to corners of prompt window
  Paint_DrawLine(160, 62, 17, 73, GRED, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 62, 140, 73, GRED, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(160, 108, 17, 101, GRED, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 108, 140, 101, GRED, DOT_FILL_AROUND, LINE_STYLE_DOTTED);

  //partial update which updates the prompt specifically
  //need to move this and the prompt itself off to the side
  //TODO: changed the dimensions of this partial update
  LCD_2IN_DisplayWindows(59, 6, 111, 145, s_buffer);
}


inline static void game_UI(){
   Paint_SelectImage((UBYTE *)s_buffer);

   if (!fired){
      clearflags();
      game_state = true;
      Paint_Clear(BLACK);
      write_prompt(action);
      populate_UI_elements(value, score_d);
      LCD_2IN_Display((UBYTE *)s_buffer);
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
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  //need to add in the bomb graphic
  enclosure();

  //switch case dependent on action
  //determines which part of the graphic to highlight in green
  //0=turn it, 1=yank it, 2=wire it
  switch(action) {
    case 0:
      turn_draw(2);
      yank_draw(0);
      wire_draw(0);
    break;
    case 1:
      turn_draw(0);
      yank_draw(2);
      wire_draw(0);
    break;
    case 2:
      turn_draw(0);
      yank_draw(0);
      wire_draw(2);
    break;
    default:
    break;
  }


  //need to shift this string to the left side
  Paint_DrawString_EN(12, 77, "CORRECT", &Font20, GREEN, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

 inline static void incorrect_disp(){  //BEING CHANGED
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  Paint_DrawString_EN (110, 100, "BOOM", &Font20, RED, BLACK);
  //add multiple explosion graphics in the form of concentric circles
  //use a switch case depending on index
  //start at case index=9, work down to 0
  //no break statements

  //ZONE OF EXCLUSION FOR EXPLOSIONS
  //X COORDS: 110 TO 197
  //Y COORDS: 100 TO 120
  //Assuming the radfactor is no greater than 1:
  //x_cen can not be between 62 and 245
  //x_cen can not be less than 49 or greater than 272
  //y_cen can not be between 52 and 168
  //y_cen can not be less than 48 or greater than 192
  //center coordinates in that range will draw over the text
  switch(index){
    case 9:
      //in each case, add an explosion_draw
      //need to figure out coordinates for each explosion, as well as radius factor
      //gonna just guess the coords and not change radius factor for now
      explosion_draw(262, 57, 1);
    case 8:
      explosion_draw(134, 40, 1);
    case 7:
      explosion_draw(132, 186, 1);
    case 6:
      explosion_draw(265, 183, 1);
    case 5:
      explosion_draw(56, 113, 1);
    case 4:
      explosion_draw(68, 51, 1);
    case 3:
      explosion_draw(218, 169, 1);
    case 2:
      explosion_draw(205, 49, 1);
    case 1:
      explosion_draw(77, 174, 1);
    case 0:
      explosion_draw(268, 108, 1);
    default:
    break;
  }

  LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void play_again(){
  if (!fired)
  {
    clearflags();
    restart_state = true;
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_Clear(BLACK);
    Paint_DrawString_EN(110, 100, "PLAY AGAIN?", &Font20, GREEN, BLACK);
    LCD_2IN_Display((UBYTE *)s_buffer);
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

  // if(str_idx >= 1024){
  //   printf("Buffer full, hit delete or numlock->insert to clear\n");
  //   return;
  // }


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
          char base_str[32];
          sprintf(base_str,"Base changed to %d\n", base);
          Paint_DrawString_EN(5, 5, base_str, key_text.font_size, key_text.color, key_text.background);
        }
        else if(*endptr == '\0'){
          Paint_DrawString_EN(5, 5, "Invalid base.", key_text.font_size, key_text.color, key_text.background);
          //printf("Invalid base\n");
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
              //op1 = pow(op1, op2);
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

    // printf("%c%c%c%c", 0x1B, 0x5B, 0x32, 0x4A); //This clears the serial terminal
    // printf("%s    %u %u", str_buffer, str_idx,calc_idx);
    tone(&tone_gen, NOTE_A3, 100);

    if(str_idx >= 5){
      if(str_buffer[str_idx - 5] == '.' && str_buffer[str_idx - 4] == '.' && 
         str_buffer[str_idx - 3] == '.' && str_buffer[str_idx - 2] == '.' && 
         str_buffer[str_idx - 1] == '.'){
        calculator_mode = !calculator_mode;
        str_idx = 0;
        str_buffer[str_idx] = '\0';
        if(!calculator_mode){
        // printf("Calculator mode disengaged.\n");
        Paint_DrawString_EN(5, 5, "Calculator mode disengaged.", key_text.font_size, key_text.color, key_text.background);

        melody(&tone_gen, CONFIRM, 1);
        }
        else{
        // printf("Calculator mode engaged.\n");
        Paint_DrawString_EN(5, 5, "Calculator mode engaged.", key_text.font_size, key_text.color, key_text.background);
        melody(&tone_gen, REJECT, 1);
        }
      }
    }
  
   Paint_ClearWindows(0, 0, 320, 18, BLACK);
   Paint_DrawString_EN(5, 5, str_buffer, key_text.font_size, key_text.color, key_text.background);
   LCD_2IN_Display((uint8_t *)s_buffer);
}


// inline void correct_disp(void);

// inline void incorrect_disp(void);

// inline void end_disp(int score);

// inline void restart_disp(void);

#endif 