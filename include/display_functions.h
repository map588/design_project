#ifndef DISPLAY_FUNCTIONS
#define DISPLAY_FUNCTIONS

#include "display_manager.h"
#include "text_properties.h"
#include <stdint.h>


bool init_display();


inline void clearflags(){
  select_state = false;
  game_state = false;
  load_state = false;
}


 static inline void select_display (uint8_t key){
  // The positions of the arrow when in select state
  const int arr_pos[3] = {64, 152, 240};
  static uint8_t last_key;
  
  Paint_SelectImage ((UBYTE *)s_buffer);

  
  if(last_key > 0 && key == 0)
    last_key--;
  else 
  if(last_key < 2 && key == 1)
    last_key++;

  
   if(!select_state){
    last_key = 0;
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
   }
   Paint_ClearWindows(0, 200, 319, 239, BLACK); // bottom bar
   int arrow_pos = arr_pos[last_key];

   Paint_DrawString_EN(arrow_pos, 210, "^", &Font20, WHITE, BLACK);


   LCD_2IN_Display((UBYTE *)s_buffer);
}


  void countdown_bar(uint8_t index){
  uint16_t x1;
  uint16_t x2;
  uint16_t y1;
  uint16_t y2;
  y1 = load_properties.y1;
  y2 = load_properties.y2;
 if (index == 0)
  {
  Paint_SelectImage((UBYTE *)s_buffer);
  Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
  x1 = load_properties.x1;
  x2 = load_properties.x1 + load_properties.width - 5;
  } else {
  x1 = load_properties.x1 + load_properties.width * index;
  x2 = load_properties.x1 + load_properties.width * (index + 1) - 5;
  } 
  
  Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);

  if(index == 9){
  LCD_2IN_Display((UBYTE *)s_buffer);
  Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
  }
  LCD_2IN_Display((UBYTE *)s_buffer);
}


  void populate_UI_elements(uint16_t countdown, uint8_t score){
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 0, 319, 29, BLACK);   // top bar
    Paint_ClearWindows(0, 220, 80, 239, BLACK); // approximately time window
    uint8_t round = score / 20;
    uint8_t n_round = 20 - (score % 20);

    // All have an extra 2 bytes for a null and a mistake
    char score_str[14]; // 8 + 1 + 2 + 1 + 2 = 14
    char round_str[14]; // 8 + 1 + 2 + 1 + 2 = 14
    char nextR_str[14]; // 8 + 1 + 2 + 1 + 2 = 14
    char time_str[18];  // 8 + 1 + 4 + 1 + 2 + 2 = 18

    sprintf(score_str, "%s %u", UI_Text[SCORE].text, score);
    sprintf(round_str, "%s %u", UI_Text[ROUND].text, round);
    sprintf(nextR_str, "%s %u", UI_Text[NEXT].text, n_round);
    sprintf(time_str, "%s %u ms", UI_Text[TIME].text, countdown);

    Paint_DrawString_EN(UI_Text[SCORE].x, UI_Text[SCORE].y, score_str,
                        UI_Text[SCORE].font_size, UI_Text[SCORE].color, UI_Text[SCORE].background);

    Paint_DrawString_EN(UI_Text[ROUND].x, UI_Text[ROUND].y, round_str,
                        UI_Text[ROUND].font_size, UI_Text[ROUND].color, UI_Text[ROUND].background);

    Paint_DrawString_EN(UI_Text[NEXT].x, UI_Text[NEXT].y, nextR_str,
                        UI_Text[NEXT].font_size, UI_Text[NEXT].color, UI_Text[NEXT].background);

    Paint_DrawString_EN(UI_Text[TIME].x, UI_Text[TIME].y, time_str,
                        UI_Text[TIME].font_size, UI_Text[TIME].color, UI_Text[TIME].background);

    LCD_2IN_Display((UBYTE *)s_buffer);
}


 int write_prompt(uint8_t action){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 90, 319, 150, BLACK); //rough prompt window

  Paint_DrawString_EN (UI_Prompt[action].x, UI_Prompt[action].y, UI_Prompt[action].text,
   UI_Prompt[action].font_size, UI_Prompt[action].color, UI_Prompt[action].background);

  Paint_DrawRectangle (UI_Prompt[action].x - 5, UI_Prompt[action].y - 5,
   UI_Prompt[action].x + UI_Prompt[action].text_length + 5, UI_Prompt[action].y + 19,
   UI_Prompt[action].color, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

  LCD_2IN_Display((UBYTE *)s_buffer);
}

inline void correct_disp(){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 239, BLACK);
  Paint_DrawString_EN (110, 100, "CORRECT", &Font20, GREEN, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
  }

inline void incorrect_disp(){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 239, BLACK);
  Paint_DrawString_EN (110, 100, "INCORRECT", &Font20, RED, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
  }

  static inline void drive_hex(uint8_t hex){
    gpio_put(hex_0,  hex & 0x01);
    gpio_put(hex_1, (hex & 0x02) >> 1);
    gpio_put(hex_2, (hex & 0x04) >> 2);
    gpio_put(hex_3, (hex & 0x08) >> 3);
  }

 static inline void game_UI(uint16_t countdown, uint8_t score, uint8_t index, uint8_t action){
  if(!game_state && load_state){
    clearflags();
    game_state = true;
  }else if(!game_state){
    clearflags();
    Paint_Clear(BLACK);
    game_state = true;
  }

  if(index == 0){
  Paint_SelectImage ((UBYTE *)s_buffer);
  write_prompt(action);
  populate_UI_elements(countdown, score); 
  }


  drive_hex(index);
  countdown_bar(index);
}



inline void displayPacket( uint16_t value, uint8_t score, uint8_t index, uint8_t action, uint8_t state){

  char packet_s[23];
  char action_s[5];
  char  state_s[5];

  const char *state_str[7] = {"LOAD", "SEL ", "GAME", "PASS", "FAIL", "RST ", "KEY "};
  const char *action_str[4] = {"TURN", "WIRE", "YANK", "NOP "};

  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(30, 31, 240, 54, BLACK);

  sprintf(action_s, "%s", action_str[action]);
  sprintf(state_s, "%s", state_str[state]);
  sprintf(packet_s, "%u_%u #%u %s %s", value, score, index, action_s, state_s);
  Paint_DrawString_EN (30, 32, packet_s, &Font12, WHITE, BLACK);
  packet_s[0] = '\0';
  LCD_2IN_Display((UBYTE *)s_buffer);
}


 static inline void display_key(uint8_t character){

  static int  str_idx = 0;
  static char str_buffer[256];

  if(!key_state || !load_state){
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

  Paint_ClearWindows(0, 0, 320, 18, BLACK);
  Paint_DrawString_EN(5, 5, str_buffer, key_text.font_size, key_text.color, key_text.background);

  LCD_2IN_Display((uint8_t *)s_buffer);

}

#endif 


