
#include "pico/stdlib.h"
#include "display_functions.h"


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


static uint16_t value = 1000;
static uint8_t action = 0;
static uint8_t score = 0;
static states state = LOADING;


static repeating_timer_t idx_timer;
static alarm_pool_t *core1_pool;
static uint8_t index = 0;
static uint8_t last_index = 0;

 uint16_t *s_buffer;

bool idx_timer_callback(repeating_timer_t *rt){
  last_index = index;
  index++;

  if(index > 9){
    index = 0;
    return false;
  }

  gpio_put(PICO_DEFAULT_LED_PIN, 1);
  
  return true;
}


//This is mostly stuff from the display library that I copied to get it to work
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
  if(!alarm_pool_add_repeating_timer_ms(core1_pool, 0, idx_timer_callback, NULL, &idx_timer)){
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
  gpio_set_dir (25, GPIO_OUT);

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
    interrupt = 1;

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
  while(1){

    if (last_index != index)
    {
      switch (state)
        {
        case SELECT:
          select_display(index);
          break;
        case LOADING:
          countdown_bar(index);
          
          break;
        case GAME:
          game_UI(value, score, index, action);
          break;
        case KEYPRESS:
          display_key(score);
        case CORRECT:
            correct_disp();
            break;
        case INCORRECT:
            incorrect_disp();
            break;
        // case RESTART:
        //     restart_disp();
        //     break;
        default:
          break;
        }
        last_index = index;
        LCD_2IN_Display((uint8_t *)s_buffer);
    }
  }
  if(last_index == index){
    gpio_put(25, 0);
  }

  // This is for debugging, it displays the contents of the packet sent by core 0 on every interrupt
  if(interrupt){
    interrupt = 0;
    displayPacket(value, score, index, action, state);
  }

  while (last_index == index)
    tight_loop_contents();
}

void display_exit (){

  free (s_buffer);
  s_buffer = NULL;

  DEV_Module_Exit ();
  return;
}