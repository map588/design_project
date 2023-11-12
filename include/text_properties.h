#include "../lib/GUI/GUI_Paint.h"
#include <stdint.h>
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

enum prompt {ARM, REWIRE, YANK};

const text_properties UI_Prompt[3] = {
  {  62,  110, GRED  , BLACK, &Font20, "   TURN IT   ", 196},  //bottom left
  {  62,  110, GRED  , BLACK, &Font20, "   WIRE IT   ", 196},  //bottom left
  {  62,  110, GRED  , BLACK, &Font20, "   YANK IT   ", 196},  //bottom left
};

const text_properties key_text = {
    2, // Border on 0,0
    2, // ^
    WHITE,
    BLACK,
    &Font16,   // Text is 11 wide, 16 tall
    "FILLER_", // Dummy text, we are concerned with the other members
    13         // Width + 2 offset pixels
};