
#include "pico/stdlib.h"
//#include "display_functions.h"
#include "display_manager.h"
#include "text_properties.h"
#include <stdint.h>

static uint16_t value = 1000;
static uint8_t action = 0;
static uint8_t score = 0;
static states state = LOADING;

static repeating_timer_t idx_timer;
static alarm_pool_t *core1_pool;
static uint8_t index = 0;

static uint16_t *s_buffer;

typedef void (*function_ptr)(void);
typedef struct{
  function_ptr func;
  bool repeating;
}function_holder;



bool init_display();


 void clearflags(){
  select_state = false;
  game_state = false;
  load_state = false;
}

 static void select_display (){

  const int arr_pos[3] = {64, 152, 240};
  static uint8_t last_key = 0;
 
  uint8_t key = action;
  if(last_key > 0 && key == 0)
    last_key--;
  else 
  if(last_key < 2 && key == 1)
    last_key++;

  Paint_SelectImage((UBYTE *)s_buffer);
  if (!select_state){
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

  void countdown_bar(){
    uint8_t bar_idx = index;
        uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    y1 = load_properties.y1;
    y2 = load_properties.y2;
  if (bar_idx == 0){
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
    x1 = load_properties.x1;
    x2 = load_properties.x1 + load_properties.width - 5;
    } 
    else {
    x1 = load_properties.x1 + load_properties.width * bar_idx;
    x2 = load_properties.x1 + load_properties.width * (bar_idx + 1) - 5;
    } 
    
    Paint_DrawRectangle(x1, y1, x2, y2, WHITE, DOT_FILL_AROUND, DRAW_FILL_FULL);

    if(bar_idx == 9){
    LCD_2IN_Display((UBYTE *)s_buffer);
    Paint_ClearWindows(load_properties.x1 - 15, load_properties.y1 - 2, 319, 239, BLACK);
    }

    LCD_2IN_Display((UBYTE *)s_buffer);
}


  void populate_UI_elements(){
    uint16_t countdown = value;
    uint8_t score = score;
 
    Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 0, 319, 29, BLACK);   // top bar
    Paint_ClearWindows(0, 220, 80, 239, BLACK); // approximately time window
    uint8_t round = score / 20;
    uint8_t n_round = 20 - (score % 20);

    char score_str[14]; 
    char round_str[14]; 
    char nextR_str[14]; 
    char time_str[18];  

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


  void write_prompt(){

        Paint_SelectImage((UBYTE *)s_buffer);
    Paint_ClearWindows(0, 90, 319, 150, BLACK); // rough prompt window

    Paint_DrawString_EN(UI_Prompt[action].x, UI_Prompt[action].y, UI_Prompt[action].text,
                        UI_Prompt[action].font_size, UI_Prompt[action].color, UI_Prompt[action].background);

    Paint_DrawRectangle(UI_Prompt[action].x - 5, UI_Prompt[action].y - 5,
                        UI_Prompt[action].x + UI_Prompt[action].text_length + 5, UI_Prompt[action].y + 19,
                        UI_Prompt[action].color, DOT_FILL_AROUND, DRAW_FILL_EMPTY);

    LCD_2IN_Display((UBYTE *)s_buffer);
}

 void correct_disp(){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 239, BLACK);
  Paint_DrawString_EN (110, 100, "CORRECT", &Font20, GREEN, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
  }

 void incorrect_disp(){
  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(0, 0, 319, 239, BLACK);
  Paint_DrawString_EN (110, 100, "INCORRECT", &Font20, RED, BLACK);
  LCD_2IN_Display((UBYTE *)s_buffer);
  }

  static void drive_hex(uint8_t hex){
    gpio_put(hex_0,  hex & 0x01);
    gpio_put(hex_1, (hex & 0x02) >> 1);
    gpio_put(hex_2, (hex & 0x04) >> 2);
    gpio_put(hex_3, (hex & 0x08) >> 3);
  }

 static  void game_UI(){
  uint16_t countdown = value;
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



 void displayPacket( uint16_t value, uint8_t score, uint8_t index, uint8_t action, uint8_t state){

  char packet_s[23];
  char action_s[5];
  char  state_s[5];

  const char *state_str[7] = {"LOAD", "SEL ", "GAME", "PASS", "FAIL", "RST ", "KEY "};
  const char *action_str[4] = {"TURN", "YANK", "WIRE", "NOP "};

  Paint_SelectImage ((UBYTE *)s_buffer);
  Paint_ClearWindows(30, 31, 240, 54, BLACK);

  sprintf(action_s, "%s", action_str[action]);
  sprintf(state_s, "%s", state_str[state]);
  sprintf(packet_s, "%u_%u #%u %s %s", value, score, index, action_s, state_s);
  Paint_DrawString_EN (30, 32, packet_s, &Font12, WHITE, BLACK);
  packet_s[0] = '\0';
  LCD_2IN_Display((UBYTE *)s_buffer);
}


 static void display_key(){

  uint8_t character = score;

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

function_holder display_functions[7] = {
    {countdown_bar, true},
    {select_display, false},
    {game_UI, true},
    {correct_disp, false},
    {incorrect_disp, false},
    {display_key, false},
    // {restart_disp}, unimplemented
};

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
//TODO: Timing is very off, need to fix pico/time.h might be necessary

//This is the core 1 stuff




bool idx_timer_callback(repeating_timer_t *rt){
  //last_index = index;
  
  displayPacket(value, score, index, action, state);
  if(display_functions[state].repeating || index == 0)
     display_functions[state].func();

  ++index;

  if(index > 9){
    return false;
  }

  gpio_put(PICO_DEFAULT_LED_PIN, 1);
  return true;
}



bool null_callback(repeating_timer_t *rt){return false;}


bool init_display()
{
  DEV_Delay_ms (100);
  if (DEV_Module_Init () != 0){
      return false;
    }
  DEV_SET_PWM (50);
  LCD_2IN_Init (HORIZONTAL);
  LCD_2IN_Clear (BLACK);

  //Noteably we create an alarm pool before we allocate essentially the rest of the memory to the display buffer
  core1_pool = alarm_pool_create(2,8);
  if(!alarm_pool_add_repeating_timer_ms(core1_pool, 0, null_callback, NULL, &idx_timer)){
    return false;
  }

  uint32_t Imagesize = LCD_2IN_HEIGHT * LCD_2IN_WIDTH * 2;

  //This massive memory allocation needs to be on the heap, but it needs to be stored globally
  //We cannot directly store it globally because it will end up in the .data section of the binary
  //So we allocate it on the heap as a constant pointer and store the pointer globally into s_buffer
  void *const buffer = malloc(Imagesize);

  if (buffer == NULL){
      printf ("Failed to allocate memory...\r\n");
      return false;
    }

  s_buffer = (uint16_t*) buffer;

  Paint_NewImage ((UBYTE *)s_buffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, 90, BLACK);
  Paint_SetScale (65);
  Paint_Clear (BLACK);
  Paint_SetRotate (ROTATE_270);


  //sets up the pins to deal with hex display
  gpio_set_dir (hex_0, GPIO_OUT);
  gpio_set_dir (hex_1, GPIO_OUT);
  gpio_set_dir (hex_2, GPIO_OUT);
  gpio_set_dir (hex_3, GPIO_OUT);
  gpio_set_dir (PICO_DEFAULT_LED_PIN, GPIO_OUT);

  gpio_pull_down (hex_0);
  gpio_pull_down (hex_1);
  gpio_pull_down (hex_2);
  gpio_pull_down (hex_3);

  return true;
}


void core_one_interrupt_handler (void){

  //While there is valid data in the interrupt FIFO
  while(multicore_fifo_rvalid ()){
      //Get value in FIFO
      uint32_t data = multicore_fifo_pop_blocking ();

      //Unpack the data
      value      =  (data   & 0xFFFF0000) >> 16;
      score      =  (data   & 0x0000FF00) >>  8;
      action     =  (data   & 0x000000F0) >>  4;
      state      =   data   & 0x0000000F;

    //actions are indexed by 1 on the other side, but they are indexed by 0 here
    if(action!= 0)
        action--;
    }
    //Cancel the previous timer
     cancel_repeating_timer(&idx_timer);

    //Calculate the new interval
    int32_t interval = ((int32_t) value) / -10;

    //Add the new timer
    alarm_pool_add_repeating_timer_ms(core1_pool, interval, idx_timer_callback, NULL, &idx_timer);

    //Reset the index, and the debug "interrupt" flag
    index = 0;

    //clear that mofo
    multicore_fifo_clear_irq();
    return;
}



//core 1 entry
void core_one_main (){
  
  //calls the above nasty function
  if (!init_display())
    exit(1);
  
  //sets up the interrupt handler for core 1, just in case
  multicore_fifo_clear_irq ();

  //sets the interrupt handler for core 1 and enables it
  irq_set_exclusive_handler (SIO_IRQ_PROC1, core_one_interrupt_handler);
  irq_set_enabled(SIO_IRQ_PROC1, true);

  //This loop just checks the global variables which are modified by the interrupt handler, and only changes if the index changes,
  //which occurs on a repeating timer, which is set by the interrupt handler, and gets updated using the "value"/10 sent by core 0
  //core 0 uses the entire interval for the timer, core 1 uses 1/10th of the interval for the timer for display updates
  // while(1){

  //    if (last_index != index)
  //    {
      // switch (state)
      //   {
      //   case SELECT:
      //     select_display();
      //     break;
      //   case LOADING:
      //     countdown_bar();
      //     break;
      //   case GAME:
      //     game_UI();
      //     break;
      //   case KEYPRESS:
      //     display_key();
      //     break;
      //   case CORRECT:
      //       correct_disp();
      //       break;
      //   case INCORRECT:
      //       incorrect_disp();
      //       break;
      //   // case RESTART:
      //   //     restart_disp();
      //   //     break;
      //   default:
      //     break;
      //   }
      //   last_index = index;
      // LCD_2IN_Display((uint8_t *)s_buffer);
  //    }
  //  }
  // if(index == 4){
  //   gpio_put(PICO_DEFAULT_LED_PIN, 0);
  // }
  // This is for debugging, it displays the contents of the packet sent by core 0 on every interrupt
  // if(interrupt){
  //   interrupt = 0;
  //   displayPacket(value, score, index, action, state);
  // }

  while (true){ tight_loop_contents();}
 }

void display_exit (){

  free (s_buffer);
  s_buffer = NULL;

  DEV_Module_Exit ();
  return;
}