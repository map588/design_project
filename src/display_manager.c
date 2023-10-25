
#include "display_manager.h"

//       _______________________________
//      |0,0                         X  |
//      |                               |
// 240  |                               |
//      |                               |
//      |                               |
//      | Y                             |
//      |_______________________________|
//                    320        

// 320 - 70 - 70 - 24 = 156 / 3 = 52
//Y - 120

static UWORD thisBuffer[IMAGE_SIZE];
//static UWORD nextBuffer[IMAGE_SIZE];

bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

bool init_display() { 
  
  DEV_Delay_ms(100);

  if (DEV_Module_Init() != 0) {
    return false;
  }

  DEV_SET_PWM(50);

  LCD_2IN_Init(HORIZONTAL);
  LCD_2IN_Clear(BLACK);

  Paint_NewImage((UBYTE *)thisBuffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 0, WHITE);
  Paint_SetScale(65);
  Paint_Clear(BLACK);

  // Paint_NewImage((UBYTE *)nextBuffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 0, WHITE);
  // Paint_SetScale(65);
  // Paint_Clear(BLACK);
  return true;
}

void start_game_display(){

  UBYTE *START = (UBYTE *)thisBuffer;

  Paint_SelectImage(START);

  Paint_DrawString_EN(80, 160, "Which One?", &Font16, WHITE, BLACK);

  Paint_DrawLine(70, 80, 70, 160, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);

  Paint_DrawLine(70 + 56, 80, 70 + 56 * 1, 160, RED, DOT_PIXEL_4X4,
                 LINE_STYLE_SOLID);

  Paint_DrawLine(70 + 56 * 2, 80, 70 + 56 * 2, 160, RED, DOT_PIXEL_4X4,
                 LINE_STYLE_SOLID);

  Paint_DrawString_EN(60, 200, "(easy)", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(60 + 56, 200, "(medium)", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(60 + 56 * 2, 200, "(hard)", &Font16, WHITE, BLACK);


  LCD_2IN_Display(START);

  // Will need to shift by 56 left or right (guarded) on arrow press
  // Paint_DrawRectangle(60, 90, 70, 170, WHITE);

}



