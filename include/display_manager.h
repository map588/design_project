#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define LCD LCD_2IN
#define hex_a  2
#define hex_b  3
#define hex_c  4
#define hex_d  5
#define hex_e  6  
#define hex_f  7
#define hex_g  8

#define buzzer 28

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

inline static void draw_mini_hex(){
  char hex_str[2];
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(151, 176, 163, 193, BLACK);
  //draw the number 8 in red on the index display
  sprintf(hex_str, "%x", (9 - index));
  Paint_DrawChar(151, 176, hex_str[0], &Font16, RED, BLACK);
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
  Paint_DrawRectangle(202, 74, 218, 82, color, DOT_FILL_AROUND, DRAW_FILL_FULL);
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
  //draw the rectangle for the index display
  draw_mini_hex();
  
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

    //new addition: pair of scissors layered on the wire the arrow is positioned under
    //NOTES TO SELF FOR MAKING THIS
    //coords of center of top circle: 33, 125, radius of 16
    //coords of center of low circle: 33, 157, radius of 16
    //beginning of gray rectangle: 4 to the right from top circle center, 9 below top circle center
    //end of gray rectangle, 62 right of start x, 12 below start y for rectangle

    //possible arrow positions: 64, 152, 240

    //im gonna hardcode it over the green wire to start, then implement it to change based on arrow position
    //Paint_DrawCircle(x_cen, y_cen, (uint16_t)(48 * radfactor), RED, DOT_FILL_AROUND, DRAW_FILL_FULL);
    Paint_DrawRectangle(arrow_pos-27, 134, arrow_pos+35, 146, GRAY, DOT_FILL_AROUND, DRAW_FILL_FULL); //x diff: -27
    Paint_DrawCircle(arrow_pos-31, 125, 16, BLUE, DOT_FILL_AROUND, DRAW_FILL_EMPTY); //x diff: -31
    Paint_DrawCircle(arrow_pos-31, 157, 16, BLUE, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

    //now i need to draw multiple lines to form the tip of the scissors
    //loop is pointless
    Paint_DrawLine(arrow_pos+37, 135, arrow_pos+37, 145, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID); //x diff: +37
    Paint_DrawLine(arrow_pos+38, 135, arrow_pos+38, 145, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+39, 135, arrow_pos+39, 145, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawLine(arrow_pos+40, 136, arrow_pos+40, 144, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+41, 136, arrow_pos+41, 144, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+42, 136, arrow_pos+42, 144, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawLine(arrow_pos+43, 137, arrow_pos+43, 143, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+44, 137, arrow_pos+44, 143, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+45, 137, arrow_pos+45, 143, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawLine(arrow_pos+46, 138, arrow_pos+46, 142, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+47, 138, arrow_pos+47, 142, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawLine(arrow_pos+48, 139, arrow_pos+48, 141, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(arrow_pos+49, 139, arrow_pos+49, 141, GRAY, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawPoint(arrow_pos+50, 140, GRAY, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    Paint_DrawPoint(arrow_pos+51, 140, GRAY, DOT_PIXEL_1X1, DOT_FILL_AROUND);

    Paint_DrawLine(arrow_pos-24, 140, 111, 140, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID); //x diff: -24

    Paint_DrawString_EN(arrow_pos, 210, "^", &Font20, WHITE, BLACK);
    //likely gonna convert this into a full update unless it slows it down a lot
    //LCD_2IN_DisplayWindows(200, 0, 239, 319, s_buffer);
    LCD_2IN_Display((UBYTE *)s_buffer);

}

inline static void prompt_start(){
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_Clear(BLACK);
  Paint_DrawString_EN(35, 112, "PRESS ENTER TO CONTINUE", &Font16, GREEN, BLACK);
  Paint_DrawRectangle(33, 110, 300, 130, GRED, DOT_FILL_AROUND, DRAW_FILL_EMPTY);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

inline static void drive_hex(){
  const uint8_t hexpins[10] = 
  { 
    0x7F, //0
    0x0C, //1
    0xB7, //2
    0x9F, //3
    0xCC, //4
    0xDA, //5
    0xFA, //6
    0x0F, //7
    0xFF, //8
    0xDF  //9
  };

    gpio_put(hexpins[index] & (0x40),hex_a);
    gpio_put(hexpins[index] & (0x20),hex_b);
    gpio_put(hexpins[index] & (0x10),hex_c);
    gpio_put(hexpins[index] & (0x08),hex_d);
    gpio_put(hexpins[index] & (0x04),hex_e);
    gpio_put(hexpins[index] & (0x02),hex_f);
    gpio_put(hexpins[index] & (0x01),hex_g);
  }
  
    
  



#endif 