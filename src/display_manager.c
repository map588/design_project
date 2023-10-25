
#include "display_manager.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

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

void init_display(){

    Paint_NewImage(WELCOME, 320, 240, 0, BLACK);
    Paint_NewImage(LOADING, 320, 240, 0, BLACK);
    Paint_NewImage(MENU, 320, 240, 0, BLACK);
    Paint_NewImage(UI, 320, 240, 0, BLACK);
    Paint_NewImage(GAME_OVER, 320, 240, 0, BLACK);
    Paint_NewImage(WIN, 320, 240, 0, BLACK);

    Paint_SelectImage(MENU);

    Paint_DrawString_EN(80, 160, "Which One?", &Font16, WHITE, BLACK);


    Paint_DrawLine(70, 80, 70, 160, GREEN, DOT_PIXEL_4x4, LINE_STYLE_SOLID);

    Paint_DrawLine(70 + 56, 80, 70 + 56 * 1, 160, RED, DOT_PIXEL_4x4, LINE_STYLE_SOLID);

    Paint_DrawLine(70 + 56 * 2, 80, 70 + 56 * 2, 160, RED, DOT_PIXEL_4x4, LINE_STYLE_SOLID);

    Paint_DrawString_EN(60, 200, "(easy)", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(60 + 56, 200, "(medium)", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(60 + 56 * 2, 200, "(hard)", &Font16, WHITE, BLACK);

    Paint_SelectImage(MENU);


    //Will need to shift by 56 left or right (guarded) on arrow press
    //Paint_DrawRectangle(60, 90, 70, 170, WHITE);

    
}



