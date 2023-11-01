#include "display_manager.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

//multipliers for adding to the FIFO packet
//  V = value, A = action (enum), P = Points, I = index, S = state (enum)
//  packet =>  0xVVVAPPIS
#define INDEX  0x00000010
#define SCORE  0x00000100
#define VALUE  0x00100000
#define EDGE_LOW_MASK  0x4444
#define EDGE_HIGH_MASK 0x8888


// TODO: Decide if we want to store more values as globals and assemble them in the irq handler
uint32_t packet;
states state;
bool user_input = false;

int key0 = 16;
int key1 = 17;
int key2 = 18;
int key3 = 19;


void action_isr(void)
{
  irq_clear(IO_IRQ_BANK0);

  uint32_t gpio = *(volatile uint32_t *)(IO_BANK0_BASE + 0x0f8);
  printf("GPIO_IRQ: %u\n", gpio);  // these aren't right
  bool key0_pressed = (bool) gpio & (1 << 3);
  bool key1_pressed = (bool) gpio & (1 << 7);
  bool key2_pressed = (bool) gpio & (1 << 11);
  bool key3_pressed = (bool) gpio & (1 << 15);


  if (key0_pressed)
  {
    packet |= ACTION_1;
  }
  else if (key1_pressed)
  {
    packet |= ACTION_2;
  }
  else if (key2_pressed)
  {
    packet |= ACTION_3;
  }
  else if (key3_pressed)
  {
    packet |= ACTION_4;
  }

  multicore_fifo_push_blocking(packet);
}


int init(void)
{

  stdio_init_all();

  gpio_set_input_enabled(key0, 1);
  gpio_set_input_enabled(key1, 1);
  gpio_set_input_enabled(key2, 1);
  gpio_set_input_enabled(key3, 1);

  gpio_pull_down(key0);
  gpio_pull_down(key1);
  gpio_pull_down(key2);
  gpio_pull_down(key3);
  
  gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);

  multicore_launch_core1(core_one_entry);

  if (!init_display())
  {
    exit(1);
  }
}


inline uint32_t assemble_packet(states state, uint8_t index, actions action, uint8_t score, uint16_t data)
{
  uint32_t packet = 0;
  packet |= state;
  packet |= action;
  packet |= index   * INDEX;
  packet |= score   * SCORE;
  packet |= data    * VALUE;
  return packet;
}

void loading_disp(uint16_t countdown, uint8_t iterations){
  uint16_t interval = countdown / 10;
  for(int j = 0; j < iterations; j++){
    for (int i = 0; i < 10; i++)
    {
      packet = assemble_packet(LOADING, i, 0, 0, countdown);
      multicore_fifo_push_blocking(packet);
      busy_wait_ms(interval);
    }
  }
}

void main_disp(uint16_t countdown, uint8_t score)
{
  uint16_t interval = countdown / 10;
    for (int i = 0; i < 10; i++)
    {
      packet = assemble_packet(GAME, i, 0, 0, countdown);
      multicore_fifo_push_blocking(packet);
      busy_wait_ms(interval);
    }
}

int main(void)
{
  init();

  int16_t time_rate = 0;

  int16_t time = 500;
  uint8_t score;
  uint8_t index;
  actions action;

  state = LOADING;
  
  loading_disp(time, 2);
 
  do{
    //Wait for select


  }while(1);
  
  //500ms load

  //Game Logic

  //Press Enter to start

  do{


  }while(1);

  //generate random number and pass to core 1
  //wait for response
  //if correct, increment score and time rate
  //if incorrect, decrement score and time rate




  display_exit();

  while(1){tight_loop_contents();}

  return 0;
}
