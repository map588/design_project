
#include "display_manager.h"
#include "pico/stdlib.h"

//    0,0             320              X
//       _______________________________
//      | ${score}            STAGE {n} |
//      |                               |
//      |                               |
//  240 |                               |
//      |                               |
//      |                               |
//   Y  |_______________________________|
//

uint16_t *s_buffer;
uint8_t state;

typedef struct
{
  uint16_t x1;
  uint16_t y1;
  uint16_t y2;
  uint16_t width;
} bar_properties;

bar_properties loading_properties(){
  uint16_t x1 = 215;
  uint16_t y1 = 225;
  uint16_t y2 = 235;
  uint8_t width = 10;
};


uint16_t *const alloc_const_buffer (UDOUBLE size){
uint16_t *const buffer = (UWORD *)malloc (size);
return buffer;
}


bool init_display (void){
  DEV_Delay_ms (100);
  if (DEV_Module_Init () != 0){
      return false;
    }
  DEV_SET_PWM (50);

  LCD_2IN_Init (HORIZONTAL);
  LCD_2IN_Clear (WHITE);

  UDOUBLE Imagesize = LCD_2IN_HEIGHT * LCD_2IN_WIDTH * 2;
  uint16_t *const buffer = alloc_const_buffer (Imagesize);

  if (buffer == NULL){
      printf ("Failed to apply for black memory...\r\n");
      exit (0);
    }
  s_buffer = buffer;
  // /*1.Create a new image cache buffer and fill it with white*/
  Paint_NewImage ((UBYTE *)s_buffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 90, WHITE);
  Paint_SetScale (65);
  Paint_Clear (WHITE);
  Paint_SetRotate (ROTATE_270);

  return true;
}

void select_display (select key){

  LCD_2IN_Clear(BLACK);

  Paint_SelectImage ((UBYTE *)s_buffer);


  Paint_DrawString_EN (110, 40, "Which One?", &Font16, WHITE, BLACK);
  Paint_DrawString_EN ( 60, 60, "(easy)",     &Font16, WHITE, BLACK);
  Paint_DrawString_EN (112, 60, "(medium)",   &Font16, WHITE, BLACK);
  Paint_DrawString_EN (164, 60, "(hard)",     &Font16, WHITE, BLACK);


  Paint_DrawLine ( 70, 80,  70, 160, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (122, 80, 122, 160,  BLUE, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (174, 80, 174, 160,   RED, DOT_PIXEL_4X4, LINE_STYLE_SOLID);

  int arrow_pos  = (70 + 52 + key) - 2;

  Paint_DrawString_EN (arrow_pos, 170, "^", &Font16, WHITE, BLACK);

  LCD_2IN_Display ((UBYTE *)s_buffer);
}

void draw_offset_rectangle(uint8_t offset){
 bar_properties properties = loading_properties();
 uint16_t x1;
 uint16_t x2;

  if(offset == 0){
  x1 = properties.x1;
  x2 = properties.x1 + properties.width;
  } else {
  x1 = properties.x1 + properties.width * offset;
  x2 = properties.x1 + properties.width * (offset + 1);
  } 
  Paint_DrawRectangle(x1, properties.y1, x2, properties.y2, WHITE, DOT_PIXEL_4X4, DRAW_FILL_FULL);
  LCD_2IN_DisplayWindows(x1, properties.y1, 239, 319, (uint8_t *)s_buffer);
}

void countdown_bar(uint16_t countdown){
  uint16_t interval = countdown / 10;
  for(int i = 0; i < 10; i++){
    draw_offset_rectangle(i);
    DEV_Delay_ms(interval);
  }
}

void game_UI(uint16_t countdown, uint8_t score, uint8_t stage){
  Paint_SelectImage ((UBYTE *)s_buffer);
  countdown_bar(countdown);
}

void core_one_interrupt_handler (void){

  while (multicore_fifo_rvalid ()){
      uint32_t data = multicore_fifo_pop_blocking ();

      
      uint16_t value   =  data   & 0xFFFF0000;
      uint8_t  score   =  data   & 0x0000FF00;
      uint8_t  stage   =  data   & 0x000000F0;
      states   state   =  data   & 0x0000000F;

      value >>= 16;
      score >>= 8;
      stage >>= 4;

      switch (state)
        {
        case SELECT:
          select_display(stage);
          break;
        case LOADING:
          countdown_bar(value);
          // oading_disp((data & 0x0F) / 10.0);
          break;
        // case GAME:
        //     game_disp(score, stage, value);
        //     break;
        // case CORRECT:
        //     correct_disp();
        //     break;
        // case INCORRECT:
        //     incorrect_disp();
        //     break;
        // case RESTART:
        //     restart_disp();
        //     break;
        default:
          break;
        }
      LCD_2IN_Display ((uint8_t *)s_buffer);
    }

  multicore_fifo_clear_irq ();
}

void core_one_entry (void){

  multicore_fifo_clear_irq ();
  irq_set_exclusive_handler (SIO_IRQ_PROC1, core_one_interrupt_handler);
  irq_set_enabled (SIO_IRQ_PROC1, true);

  // pico specific efficent sleep function to call in a "tight_loop"
  while (1){
      tight_loop_contents ();
    }
}

void display_exit (){

  free (s_buffer);
  s_buffer = NULL;

  DEV_Module_Exit ();
  return;
}