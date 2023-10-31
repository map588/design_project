
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

const bar_properties load_properties = {
220, //start x
225, //start y
235, //end y
 10, //width
     //end x is start x + width * offset
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
  LCD_2IN_Clear (BLACK);

  UDOUBLE Imagesize = LCD_2IN_HEIGHT * LCD_2IN_WIDTH * 2;
  uint16_t *const buffer = alloc_const_buffer (Imagesize);

  if (buffer == NULL){
      printf ("Failed to apply for black memory...\r\n");
      exit (0);
    }
  s_buffer = buffer;
  // /*1.Create a new image cache buffer and fill it with white*/
  Paint_NewImage ((UBYTE *)s_buffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 90, BLACK);
  Paint_SetScale (65);
  Paint_Clear (BLACK);
  Paint_SetRotate (ROTATE_270);

  return true;
}



void select_display (uint8_t key){

  LCD_2IN_Clear(BLACK);
  Paint_SelectImage ((UBYTE *)s_buffer);


  Paint_DrawString_EN (110, 25, "Which Wire?",&Font16, WHITE, BLACK);
  Paint_DrawString_EN ( 44, 55, "(easy)",     &Font12, WHITE, BLACK);
  Paint_DrawString_EN (124, 55, "(medium)",   &Font12, WHITE, BLACK);
  Paint_DrawString_EN (224, 55, "(hard)",     &Font12, WHITE, BLACK);


  Paint_DrawLine ( 70, 80,  70, 200, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (158, 80, 158, 200,  BLUE, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (246, 80, 246, 200,   RED, DOT_PIXEL_4X4, LINE_STYLE_SOLID);

  int pos[3] = {70, 158, 246};

  int arrow_pos  = pos[key%3] - 6;

  Paint_DrawString_EN (arrow_pos, 210, "^", &Font20, WHITE, BLACK);

  LCD_2IN_Display ((UBYTE *)s_buffer);
}




void draw_loading_rectangle(uint8_t offset){
 uint16_t x1;
 uint16_t x2;
 uint16_t y1;
 uint16_t y2;
  if(offset == 0){
  x1 = load_properties.x1;
  x2 = load_properties.x1 + load_properties.width - 5;
  } else {
  x1 = load_properties.x1 + load_properties.width * offset;
  x2 = load_properties.x1 + load_properties.width * (offset + 1) - 5;
  } 
  y1 = load_properties.y1;
  y2 = load_properties.y2;
  Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  LCD_2IN_Display((UBYTE *)s_buffer);
  //LCD_2IN_DisplayWindows(load_properties.y1 - 2, load_properties.x1 - 15, 319, 239, s_buffer);
}

void countdown_bar(uint16_t countdown){
  Paint_SelectImage ((UBYTE *)s_buffer);
  //Paint_Clear(BLACK);
  Paint_ClearWindows(load_properties.y1 - 15, load_properties.x1 -2, 319, 239, BLACK);
  uint16_t interval = countdown / 10;
  for(int i = 0; i < 10; i++){
    draw_loading_rectangle(i);
    busy_wait_ms(interval);
  }
  Paint_ClearWindows(load_properties.y1 - 15, load_properties.x1 - 2, 319, 239, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
}

void game_UI(uint16_t countdown, uint8_t score, uint8_t stage){
  Paint_SelectImage ((UBYTE *)s_buffer);
  countdown_bar(countdown);
}

void core_one_interrupt_handler (void){

  if(multicore_fifo_rvalid ()){
      uint32_t data = multicore_fifo_pop_blocking ();

      
      uint32_t value   =  data   & 0xFFF00000;
      uint16_t score   =  data   & 0x0000FF00;
      actions  action  =  data   & 0x000F0000;
      stage    stage   =  data   & 0x000000F0;
      states   state   =  data   & 0x0000000F;

      value >>= 20;
      score >>= 8;
      
      uint8_t idx_action = (action >> 16) - 1;


      switch (state)
        {
        case SELECT:
          select_display(idx_action);
          break;
        case LOADING:
          countdown_bar(value);
          busy_wait_ms(200);
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

  multicore_fifo_drain ();
  multicore_fifo_clear_irq ();
}

void core_one_entry (void){

  multicore_fifo_clear_irq ();
  irq_set_exclusive_handler (SIO_IRQ_PROC1, core_one_interrupt_handler);
  irq_set_enabled (SIO_IRQ_PROC1, true);

  //irq_set_exclusive_handler (TIMER_IRQ_0, core_one_interrupt_handler);

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