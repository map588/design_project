
//#include "display_functions.h"
#include "display_manager.h"


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

static bool fired;



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
  core1_pool = alarm_pool_create(2,4);
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

//Returning true in a repeating timer callback means to continue the timer
bool idx_timer_callback(repeating_timer_t *rt)
{
  
  displayPacket(value, score, index, action, state);
  if(display_functions[state].repeating || !fired){
     display_functions[state].func();
      fired = true;
  }
  ++index;

  if(index > 9){
    uint64_t t_delta = absolute_time_diff_us(get_absolute_time(), *(absolute_time_t *)rt->user_data);
    display_time_delta(t_delta);
    return false;
  }
  //set the hex display
  gpio_put(hex_0, (bool) index & 0x01);
  gpio_put(hex_1, (bool)(index & 0x02) >> 1);
  gpio_put(hex_2, (bool)(index & 0x04) >> 2);
  gpio_put(hex_3, (bool)(index & 0x08) >> 3);

  gpio_put(PICO_DEFAULT_LED_PIN, 1);
  return true;
}

//Don't look in display_manager.h, it is a mess
//This is the interrupt handler for core 1
//The inline functions in display_manager.h are used to set the display
//The display_functions array has points to static inline functions, which are indexed using state
//This ISR sets global variables that are in display_manager.h, and starts the timer
//The index timer callback uses the state to index which function it should call, and the struct has a boolean
//To determine if it needs repeating in the index timer, like the countdown bar, etc.

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

    //Calculate the new interval, negative because we want to count from the beginning of callback execution, not the end
    int32_t interval = ((int32_t) value) / -10;
    absolute_time_t *time_delta;

    //Add the new timer
    alarm_pool_add_repeating_timer_ms(core1_pool, interval, idx_timer_callback, (void *)time_delta, &idx_timer);
    *time_delta = get_absolute_time(); //log the time for comparison

    index = 0;
    fired = false;

    //clear that mofo
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
  while (true) tight_loop_contents(); // special NOP function

 }


void display_exit (){

  free (s_buffer);
  s_buffer = NULL;

  DEV_Module_Exit ();
  return;
}