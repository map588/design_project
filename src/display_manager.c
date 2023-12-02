
//#include "display_functions.h"
#include "display_manager.h"
#include "hardware/pwm.h"

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
    {play_again, 1}
    //{random_key, false},unimplemented
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
static char *g_key;

bool null_callback(repeating_timer_t *rt){return false;}


bool init_display(){
  DEV_Delay_ms (100);

  if (DEV_Module_Init () != 0)
      return false;
  
  DEV_SET_PWM (50);
  LCD_2IN_Init (HORIZONTAL);
  LCD_2IN_Clear (BLACK);

  //Noteably we create an alarm pool before we allocate essentially the rest of the memory to the display buffer
 core1_pool = alarm_pool_create(2,8);

 if(!alarm_pool_add_repeating_timer_ms(core1_pool, 0, null_callback, NULL, &idx_timer))
    return false;
  
  tone_gen.alarm_pool = alarm_pool_create_with_unused_hardware_alarm(4);
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
  gpio_init (hex_0);
  gpio_init (hex_1);
  gpio_init (hex_2);
  gpio_init (hex_3);

  gpio_init (PICO_DEFAULT_LED_PIN);
  
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

//Returning true in a repeating timer callback means to continue the timer
bool idx_timer_callback(repeating_timer_t *rt){
  if(interrupt)
    return false;

 
  if(!fired)
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
  else 
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
 


  display_functions[state].func();
  

  drive_hex (9 - index);
  ++index;

  fired = true;

  if (index > 9 || !display_functions[state].repeating) return false;

  return true;
}

void core_one_interrupt_handler (void){
  uint32_t data;

  interrupt = true;

  
 for(int i = 0; i < 6 && multicore_fifo_rvalid(); i++)
     data = multicore_fifo_pop_blocking ();

  multicore_fifo_drain();
  value   = (data & 0xFFFF0000) >> 16;
  score_d = (data & 0x0000FF00) >> 8;
  action  = (data & 0x000000F0) >> 4;
  state   =  data & 0x0000000F;
  if(action == 0)
      action = NOP;
  else
      action--;

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
  irq_set_priority(SIO_IRQ_PROC1, 0x00);
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