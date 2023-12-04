
#include "display_functions.h"

static alarm_pool_t *core1_pool;
static repeating_timer_t idx_timer;

typedef void (*function_ptr)(void);
typedef struct{
  function_ptr func;
  bool repeating;
}function_holder;
//{LOADING, SELECT, CONTINUE, COUNTDOWN, GAME, KEYPRESS, CORRECT, INCORRECT, RANDOM_KEY, RESTART} states;
function_holder display_functions[9] = {
    {loading_bar, 1},
    {selction, 0},
    {prompt_start, 0},
    {countdown_to_start, 1},
    {game_UI, 1},
    {display_key, 0},
    {correct_disp, 0},
    {incorrect_disp, 1},
    {random_key, 0},
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


static bool interrupt;

bool null_callback(repeating_timer_t *rt){return false;}


bool init_display(){
  DEV_Delay_ms (100);

  if (DEV_Module_Init () != 0)
      return false;
  
  DEV_SET_PWM (50);
  LCD_2IN_Init (HORIZONTAL);
  LCD_2IN_Clear (BLACK);

  //Noteably we create an alarm pool before we allocate essentially the rest of the memory to the display buffer
 core1_pool = alarm_pool_create(2,16);

 if(!alarm_pool_add_repeating_timer_ms(core1_pool, 0, null_callback, NULL, &idx_timer))
    return false;
  
  tone_gen.alarm_pool = alarm_pool_create(1,8);
  tone_init(&tone_gen, buzzer);
  set_rest_duration(20);
  set_tempo(160);


  //This massive memory allocation needs to be on the heap, but it needs to be stored globally
  //We cannot directly store it globally because it will end up in the .data section of the binary
  //So we allocate it on the heap as a constant pointer and store the pointer globally into s_buffer
  uint32_t Imagesize = LCD_2IN_HEIGHT * LCD_2IN_WIDTH * 2;
  void *const buffer = malloc(Imagesize);

  if (buffer == NULL){
    printf ("Failed to allocate memory...\r\n");
    return false;
  }

  s_buffer = (uint16_t*) buffer;

  Paint_NewImage ((UBYTE *)s_buffer, LCD_2IN.WIDTH, LCD_2IN.HEIGHT, ROTATE_90, BLACK);
  Paint_SetScale (65);
  Paint_Clear (BLACK);
  Paint_SetRotate (ROTATE_270);
  Paint_SelectImage ((UBYTE *)s_buffer);
  LCD_2IN_Display   ((UBYTE *)s_buffer);

  //sets up the pins to deal with hex display
  gpio_init (hex_a);
  gpio_init (hex_b);
  gpio_init (hex_c);
  gpio_init (hex_d);
  gpio_init (hex_e);
  gpio_init (hex_f);
  gpio_init (hex_g);

  gpio_init (LED_a);
  
  gpio_set_dir (hex_a, GPIO_OUT);
  gpio_set_dir (hex_b, GPIO_OUT);
  gpio_set_dir (hex_c, GPIO_OUT);
  gpio_set_dir (hex_d, GPIO_OUT);
  gpio_set_dir (hex_e, GPIO_OUT);
  gpio_set_dir (hex_f, GPIO_OUT);
  gpio_set_dir (hex_g, GPIO_OUT);

  gpio_set_dir (LED_a, GPIO_OUT);


  gpio_pull_down (hex_a);
  gpio_pull_down (hex_b);
  gpio_pull_down (hex_c);
  gpio_pull_down (hex_d);
  gpio_pull_down (hex_e);
  gpio_pull_down (hex_f);
  gpio_pull_down (hex_g);

  gpio_pull_down (LED_a);


  return true;
}

//Returning true in a repeating timer callback means to continue the timer
bool idx_timer_callback(repeating_timer_t *rt){
  if(interrupt)
    return false;

 
  if (state == LOADING)
    loading_hex();


  
  //calls the function pointer in the struct
  display_functions[state].func();
  ++index;

  

  fired = true;
  if (index > 9 || !display_functions[state].repeating) {
    clear_hex();
    index = 0;
    return false;
  }

  return true;
}

void core_one_interrupt_handler (void){
  uint32_t data;
  states new_state;
  interrupt = true;

  if(state == NULL_STATE){
   state = LOADING;
   return;
  }

  
 for(int i = 0; i < 6 && multicore_fifo_rvalid(); i++)
     data = multicore_fifo_pop_blocking ();

  value       = (data & 0xFFFF0000) >> 16;
  score_d     = (data & 0x0000FF00) >>  8;
  action      = (data & 0x000000F0) >>  4;
  new_state   =  data & 0x0000000F;
  if(action == 0)
      action = NOP;
  else
      action--;

  //TODO I think this is a possible solution for integrating the keyboard
  //Right now its only relevant for RANDOM_KEY, RESTART, SELECT, and CONTINUE as well as the easteregg if it actually works
  if (KEYPRESS == new_state){
    switch(state){
      case CONTINUE: type_state = true; state = new_state; break;
      case SELECT:
      case COUNTDOWN:
      case GAME:
      case CORRECT:
      case INCORRECT:
      case RANDOM_KEY:
      break;
      default:
      display_functions[new_state].func();
      break; 
    }
  }
  else
    state = new_state;

  index = 0;
  fired = false;


  int32_t interval = ((int32_t) value) / -10;

  cancel_repeating_timer(&idx_timer);

  interrupt = false;
  alarm_pool_add_repeating_timer_ms(core1_pool, interval, idx_timer_callback, NULL, &idx_timer);

  multicore_fifo_clear_irq();
  return;
}


//core 1 entry
void core_one_main (){
  
  //calls the above nasty function
  if (!init_display())
      exit(1);
  
  multicore_fifo_clear_irq ();

  //sets the interrupt handler for core 1 and enables it
  irq_set_exclusive_handler (SIO_IRQ_PROC1, core_one_interrupt_handler);


  //default priority is 0x80, so this will take priority over the timer
  irq_set_priority(SIO_IRQ_PROC1, 0x40);
  irq_set_enabled(SIO_IRQ_PROC1, true);


  //This is where the magic happens
  while (true)
     tight_loop_contents(); // special NOP function

 }


void display_exit (){

  free (s_buffer);
  s_buffer = NULL;

  DEV_Module_Exit ();
  return;
}