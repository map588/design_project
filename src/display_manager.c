
//#include "display_functions.h"
#include "display_manager.h"
#include "buzzer_tones.h"
#include "hardware/pwm.h"


typedef void (*function_ptr)(void);
typedef struct{
  function_ptr func;
  uint8_t repetitions;
}function_holder;

function_holder display_functions[7] = {
    {loading_bar, 9},
    {selction, 1},
    {prompt_start, 1},
    {countdown_to_start, 3},
    {game_UI, 9},
    {display_key, 1},
    {correct_disp, 1},
    {incorrect_disp, 1},
    //{random_key, false},unimplemented
    //{game_over, false},unimplemented
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
 core1_pool = alarm_pool_create(2,4);

 if(!alarm_pool_add_repeating_timer_ms(core1_pool, 0, null_callback, NULL, &idx_timer))
    return false;
  
 
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
  LCD_2IN_Display ((UBYTE *)s_buffer);

  //sets up the pins to deal with hex display
  gpio_init (hex_0);
  gpio_init (hex_1);
  gpio_init (hex_2);
  gpio_init (hex_3);
  //gpio_init (buzzer);

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

  //gpio_set_dir (buzzer, GPIO_OUT);
  //gpio_set_function(buzzer, GPIO_FUNC_PWM);

  //pwm_config config = pwm_get_default_config();
  //pwm_set_phase_correct(&config, true);
  


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
  

  drive_hex(display_functions[state].repetitions - index);
  ++index;

  fired = true;

  if (index > display_functions[state].repetitions){
      displayPacket(value, score_d, index, action, state);
      return false;
  }

  return true;
}

void core_one_interrupt_handler (void){

  //While there is valid data in the interrupt FIFO
  while(multicore_fifo_rvalid ()){
      //Get value in FIFO
    uint32_t data = multicore_fifo_pop_blocking ();
    interrupt = true;

      //Unpack the data
      value      =  (data  & 0xFFFF0000) >> 16;
      score_d    =  (data  & 0x0000FF00) >>  8;
      action     =  (data  & 0x000000F0) >>  4;
      state      =   data  & 0x0000000F;


    //actions are indexed by 1 on the other side, but they are indexed by 0 here
    if(action == 0)
        action = NOP;
    else
        action--;

    index = 0;
    fired = false;
    
    int8_t intervals;

    intervals = display_functions[state].repetitions  + 1;

    int32_t interval = ((int32_t) value) / intervals;
    cancel_repeating_timer(&idx_timer);

    interrupt = false;
    alarm_pool_add_repeating_timer_ms(core1_pool, interval, idx_timer_callback, NULL, &idx_timer);

    
    multicore_fifo_clear_irq();
    return;
  }
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