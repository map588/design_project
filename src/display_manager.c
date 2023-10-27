
#include "display_manager.h"

//                     320              X
//       _______________________________
//      |0,0 ${score}         STAGE {n} |
//      |                               |
//      |                               |
//  240 |                               |
//      |                               |
//      |                               |
//   Y  |_______________________________|
//

static UWORD *buffer;
bool reserved_addr(uint8_t addr) {
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

bool init_display() {
  DEV_Delay_ms(100);
    if(DEV_Module_Init()!=0){
        return false;
    }
    DEV_SET_PWM(50);
   

    LCD_2IN_Init(HORIZONTAL);
    LCD_2IN_Clear(WHITE);
    
    UDOUBLE Imagesize = LCD_2IN_HEIGHT*LCD_2IN_WIDTH*2;

    if((buffer = (UWORD *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        exit(0);
    }

    // /*1.Create a new image cache named buffer and fill it with white*/
    Paint_NewImage((UBYTE *)buffer,LCD_2IN.WIDTH,LCD_2IN.HEIGHT, 90, WHITE);
    Paint_SetScale(65);
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_270);
    printf("drawing...\r\n");

    return true;
}


void select_display(UWORD *buffer, select key) {


  Paint_SelectImage((UBYTE *)buffer);

  Paint_DrawString_EN(80, 160, "Which One?", &Font16, WHITE, BLACK);

  Paint_DrawLine(70, 80, 70, 160, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);

  Paint_DrawLine(70 + 56, 80, 70 + 56 * 1, 160, RED, DOT_PIXEL_4X4,
                 LINE_STYLE_SOLID);

  Paint_DrawLine(70 + 56 * 2, 80, 70 + 56 * 2, 160, RED, DOT_PIXEL_4X4,
                 LINE_STYLE_SOLID);

  Paint_DrawString_EN(60, 200, "(easy)", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(60 + 56, 200, "(medium)", &Font16, WHITE, BLACK);
  Paint_DrawString_EN(60 + 56 * 2, 200, "(hard)", &Font16, WHITE, BLACK);

  LCD_2IN_Display((UBYTE *)buffer);

  // Will need to shift by 56 left or right (guarded) on arrow press
  // Paint_DrawRectangle(60, 90, 70, 170, WHITE);

  busy_wait_ms(1999);
  DEV_Delay_ms(2000);

free(buffer);
buffer = NULL;

  DEV_Module_Exit();
}







