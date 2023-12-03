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
#include "pwm-tone.h"
#include "melodies.h"
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
bool incorrect_state = 0;


static uint16_t *s_buffer;

inline static void clearflags(){
      select_state = false;
      game_state = false;
      load_state = false;
      key_state = false;
      restart_state = false;
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
  Paint_DrawCircle(168, 38, 8, RED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
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
  Paint_DrawCircle(168, 38, 8, GREEN, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(151, 38, 187, 56, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawLine(160, 38, 160, 47, GREEN, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  }
  else //default colors
  {
  Paint_DrawCircle(168, 38, 8, WHITE, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  Paint_DrawRectangle(151, 38, 187, 56, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, BLUE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(160, 47, 6, GREEN, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawLine(160, 38, 160, 47, WHITE, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  }

  //TODO: partial update for wire
  LCD_2IN_DisplayWindows(24, 151, 56, 187, s_buffer);

}

inline static void enclosure(int8_t prompt, bool correct){
  const uint16_t colors[4] = {BLACK, RED, BLUE, GREEN};
  Paint_SelectImage((UBYTE *)s_buffer);
  //Dim the color on the screen
  uint16_t color = colors[prompt + 1] & 0x3333;
  //clear area for enclosure
  Paint_ClearWindows(143, 35, 283, 202, BLACK);
  //draw the big rectangle for the enclosure
  Paint_DrawRectangle(146, 38, 280, 198, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the rectangle for the "display"
  Paint_DrawRectangle(160, 62, 266, 108, color, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw an even tinier bomb in the display with an even tinier display
  Paint_DrawRectangle(200, 70, 220, 100, MAGENTA, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(160, 62, 266, 108, color, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the rectangle for the keypad
  Paint_DrawRectangle(177, 117, 248, 187, GRAY, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //add greater detail to keypad to reflect number of keys
  //vertical lines for keypad
  Paint_DrawLine(191, 117, 191, 187, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  Paint_DrawLine(205, 117, 205, 187, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  Paint_DrawLine(219, 117, 219, 187, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  Paint_DrawLine(233, 117, 233, 187, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  //horizontal lines for keypad
  Paint_DrawLine(177, 134, 248, 134, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  Paint_DrawLine(177, 151, 248, 151, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  Paint_DrawLine(177, 168, 248, 168, BLACK, DOT_FILL_AROUND, LINE_STYLE_SOLID);
  //draw the rectangle for the hex display
  Paint_DrawRectangle(151, 176, 163, 193, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //draw the number 8 in red on the hex display
  Paint_DrawString_EN(151, 176, "8", &Font16, RED, BLACK);

  //partially update the display for the enclosure
  //TODO: partial update
  LCD_2IN_DisplayWindows(35, 143, 202, 283, s_buffer);

  if(correct && prompt != -1) prompt += 3;

  switch (prompt)
  {
    case -1:
      turn_draw(0);
      yank_draw(0);
      wire_draw(0);
    break;
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
    case 3:
      turn_draw(2);
      yank_draw(0);
      wire_draw(0);
    break;
    case 4:
      turn_draw(0);
      yank_draw(2);
      wire_draw(0);
    break;
    case 5:
      turn_draw(0);
      yank_draw(0);
      wire_draw(2);
    break;
    default:
    break;
    }

  
}


inline static void explosion_draw(int x_cen, int y_cen, double radfactor){

  //create layered circles
  //order: red->orange->yellow->white
  Paint_DrawCircle(x_cen, y_cen, (uint16_t)(48 * radfactor), RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(x_cen, y_cen, (uint16_t)(32 * radfactor), YELLOW, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawCircle(x_cen, y_cen, (uint16_t)(16 * radfactor), WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  //Paint_DrawChar(x_cen - 7, y_cen - 6, '*', &Font20, WHITE, BRRED);  //this draws a star in the middle of the explosion

  //need to partially update ONLY the area around the circle
  //take x_cen and y_cen, subtract both by 48*radfactor to get starting coords
  //add both by 48*radfactor to get end coords
  int adjustedrad = (uint16_t)(48*radfactor) + 2;

  // int x1 = (x_cen - adjustedrad > 0)? x_cen - adjustedrad : 0;
  // int x2 = (x_cen + adjustedrad < 319)? x_cen + adjustedrad : 319;
  // int y1 = (y_cen - adjustedrad > 0)? y_cen - adjustedrad : 0;
  // int y2 = (y_cen + adjustedrad < 239)? y_cen + adjustedrad : 239;

  //LCD_2IN_DisplayWindows(y1, x1, y2, x2, s_buffer); // experimenting with just updating the whole display
  LCD_2IN_Display((UBYTE *)s_buffer);
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
        Paint_DrawLine(158, 80, 158, 200, YELLOW, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
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
  if(!fired){
    clearflags();
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_Clear(BLACK);
    LCD_2IN_Display((UBYTE *)s_buffer);
    fired = true;
  }

  //draw the bomb here, then do the switch case
  //move the countdown to the side, bomb on the right
  


  switch(index){ 
    case 0:
      enclosure(-1,0);
      break;
    case 2: //This is intentional, I want the bomb to be drawn on the screen for 2 frames
      Paint_DrawString_EN(75, 83, "3", &Font20, GREEN, BLACK);
      break;
    case 3:
      Paint_DrawString_EN(75, 83, "2", &Font20, GREEN, BLACK);
      break;
    case 4:
      Paint_DrawString_EN(75, 83, "1", &Font20, GREEN, BLACK);
      break;
    case 5:
      Paint_DrawString_EN(3, 83, "DEFUSE IT!", &Font20, RED, BLACK);
      break;
    case 6:
      index = 10;
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
  const uint16_t colors[3] = {RED, BLUE, GREEN};
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(6, 59, 145, 111, BLACK); // rough prompt window

  //draw the bomb graphic on screen
  enclosure(action, 0);

  //trying out packing all of the graphics into one function



  //writes in the action to be done and the rectangle encompassing the string
  
  //dotted lines from corners of graphic display to corners of prompt window
  Paint_DrawLine(160, 62, 7, 42, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 62, 111, 42, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(160, 108, 7, 88, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);
  Paint_DrawLine(266, 108, 111, 88, WHITE, DOT_FILL_AROUND, LINE_STYLE_DOTTED);

 
  Paint_DrawRectangle(7, 42, 111, 88, BLACK, DOT_FILL_AROUND, DRAW_FILL_FULL);
  Paint_DrawRectangle(7, 42, 111, 88, colors[action], DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  Paint_DrawString_EN(10, 55, prompt_str[action], &Font20, colors[action], BLACK);

  //partial update which updates the prompt specifically
  //need to move this and the prompt itself off to the side
  //TODO: changed the dimensions of this partial update
  LCD_2IN_DisplayWindows(59, 6, 111, 145, s_buffer);
}




inline static void correct_disp(){
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  //need to add in the bomb graphic
  enclosure(action, 1);
  
  //TODO: I moved both switch statements into enclosure, and the boolean determines whether it should call the green or red setting



  //need to shift this string to the left side
  Paint_DrawString_EN(12, 77, "CORRECT", &Font20, GREEN, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

 inline static void incorrect_disp(){  //BEING CHANGED
    if(!fired || !incorrect_state){
      clearflags();
      incorrect_state = true;
      Paint_SelectImage((UBYTE *)s_buffer);
      Paint_ClearWindows(90, 80, 217, 140, BLACK);
      Paint_DrawString_EN (110, 100, "BOOM", &Font24, RED, BLACK);
      LCD_2IN_Display((UBYTE *)s_buffer);
    }
 
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
      break;
    case 8:
      explosion_draw(134, 40, 1);
      break;
    case 7:
      explosion_draw(132, 186, 1);
      break;
    case 6:
      explosion_draw(265, 183, 1);
      break;
    case 5:
      explosion_draw(56, 113, 1);
      break;
    case 4:
      explosion_draw(68, 51, 1);
      break;
    case 3:
      explosion_draw(218, 169, 1);
      break;
    case 2:
      explosion_draw(205, 49, 1);
      break;
    case 1:
      explosion_draw(77, 174, 1);
      break;
    case 0:
      explosion_draw(268, 108, 1);
      break;
    default:
    break;
  }

  //LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void play_again(){
  if (!fired)
  {
    clearflags();
    restart_state = true;
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_DrawString_EN(110, 100, "AGAIN?", &Font20, GREEN, BLACK);
    LCD_2IN_Display((UBYTE *)s_buffer);
    fired = true;
  }
    loading_bar();
}


#endif 