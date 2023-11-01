
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
      printf ("Failed to allocate memory...\r\n");
      exit (0);
    }
  s_buffer = buffer;
  // /*1.Create a new image cache buffer and fill it with black*/
  Paint_NewImage ((UBYTE *)s_buffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 90, BLACK);
  Paint_SetScale (65);
  Paint_Clear (BLACK);
  Paint_SetRotate (ROTATE_270);

  return true;
}

//The positions of the arrow when in select state
const int arr_pos[3] = {64, 152, 240};

void select_display (actions key){  //TODO: Something needs to hold the state for the arrow position

  LCD_2IN_Clear(BLACK);
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 200, 319, 239, BLACK); //bottom bar

  Paint_DrawString_EN (110, 25, "Which Wire?",&Font16, WHITE, BLACK);
  Paint_DrawString_EN ( 44, 55, "(easy)",     &Font12, WHITE, BLACK);
  Paint_DrawString_EN (124, 55, "(medium)",   &Font12, WHITE, BLACK);
  Paint_DrawString_EN (224, 55, "(hard)",     &Font12, WHITE, BLACK);


  Paint_DrawLine ( 70, 80,  70, 200, GREEN, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (158, 80, 158, 200,  BLUE, DOT_PIXEL_4X4, LINE_STYLE_SOLID);
  Paint_DrawLine (246, 80, 246, 200,   RED, DOT_PIXEL_4X4, LINE_STYLE_SOLID);

  uint8_t key_index = (key >> 16) - 1;  
  int arrow_pos  = arr_pos[key_index%3];  // %3 to prevent overflow, in case that is possible

  Paint_DrawString_EN (arrow_pos, 210, "^", &Font20, WHITE, BLACK);

  LCD_2IN_Display ((UBYTE *)s_buffer);
}




void countdown_bar(uint8_t index){
 uint16_t x1;
 uint16_t x2;
 uint16_t y1;
 uint16_t y2;
  if(index == 0){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
  x1 = load_properties.x1;
  x2 = load_properties.x1 + load_properties.width - 5;
  } else {
  x1 = load_properties.x1 + load_properties.width * index;
  x2 = load_properties.x1 + load_properties.width * (index + 1) - 5;
  } 
  y1 = load_properties.y1;
  y2 = load_properties.y2;
  Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);
  LCD_2IN_Display((UBYTE *)s_buffer);
  if(index == 9){
   Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
   LCD_2IN_Display((UBYTE *)s_buffer);
  }
}


void populate_UI_elements(uint16_t countdown, uint8_t score){
Paint_ClearWindows(0,   0, 319,  20, BLACK);    // top bar
Paint_ClearWindows(0, 220,  80, 239, BLACK);    //approximately time window
uint8_t round   =       score / 20;
uint8_t n_round = 20 - (score % 20);

//All have an extra 2 bytes for a null and a mistake
char *score_str  = malloc(14); //8 + 1 + 2 + 1 + 2 = 14
char *round_str  = malloc(14); //8 + 1 + 2 + 1 + 2 = 14
char *nextR_str  = malloc(14); //8 + 1 + 2 + 1 + 2 = 14
char *time_str   = malloc(18); //8 + 1 + 4 + 1 + 2 + 2 = 18

sprintf(score_str, "%s %u"   , UI_Text[SCORE].text , score);
sprintf(round_str, "%s %u"   , UI_Text[ROUND].text , round);
sprintf(nextR_str, "%s %u"   , UI_Text[NEXT].text  , n_round);
sprintf(time_str , "%s %u ms", UI_Text[TIME].text  , countdown);

Paint_SelectImage ((UBYTE *)s_buffer);

Paint_DrawString_EN (UI_Text[SCORE].x, UI_Text[SCORE].y, score_str, 
UI_Text[SCORE].font_size, UI_Text[SCORE].color, UI_Text[SCORE].background);

Paint_DrawString_EN (UI_Text[ROUND].x, UI_Text[ROUND].y, round_str, 
UI_Text[ROUND].font_size, UI_Text[ROUND].color, UI_Text[ROUND].background);

Paint_DrawString_EN (UI_Text[NEXT].x , UI_Text[NEXT].y , nextR_str, 
UI_Text[NEXT].font_size , UI_Text[NEXT].color , UI_Text[NEXT].background );

Paint_DrawString_EN (UI_Text[TIME].x , UI_Text[TIME].y , time_str , 
UI_Text[TIME].font_size , UI_Text[TIME].color , UI_Text[TIME].background );


LCD_2IN_Display((UBYTE *)s_buffer);

free(score_str);
free(round_str);
free(nextR_str);
free(time_str);
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



void game_UI(uint16_t countdown, uint8_t score, uint8_t index, uint8_t action){
  if(index == 0){
  Paint_SelectImage ((UBYTE *)s_buffer);
  write_prompt(action);
  populate_UI_elements(countdown, score); 
  }
  countdown_bar(index);
}




void core_one_interrupt_handler (void){
  //Allows for retriggers
  multicore_fifo_clear_irq();

  while(multicore_fifo_rvalid ()){
      uint32_t data = multicore_fifo_pop_blocking ();

      
      uint32_t value      =  data   & 0xFFF00000;
      uint16_t score      =  data   & 0x0000FF00;
      uint8_t  index      =  data   & 0x000000F0;
      actions  action     =  data   & 0x000F0000;
      states   state      =  data   & 0x0000000F;

      value >>= 20;
      score >>= 8;
      index >>= 4;

      switch (state)
        {
        case SELECT:
          select_display(action);
          break;
        case LOADING:
          countdown_bar(index);
          break;
        case GAME:
            game_UI(value, score, index, action);
            break;
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