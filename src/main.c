#include "display_manager.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

//multipliers for adding to the FIFO packet
#define SCORE  0x00000100
#define VALUE  0x00100000
#define EDGE_LOW_MASK 0b100010001000100;

uint32_t packet;

int key0 = 16;
int key1 = 17;
int key2 = 18;
int key3 = 19;


void action_isr(void)
{

  uint32_t gpio = *(volatile uint32_t *)(IO_BANK0_BASE + 0x0f8);

  printf("GPIO_IRQ: %u\n", gpio);
  bool key0_pressed = (bool) gpio & (1 << 2);
  bool key1_pressed = (bool) gpio & (1 << 6);
  bool key2_pressed = (bool) gpio & (1 << 10);
  bool key3_pressed = (bool) gpio & (1 << 14);

  if (key0_pressed)
  {
    packet |= ACTION1;
  }
  else if (key1_pressed)
  {
    packet |= ACTION2;
  }
  else if (key2_pressed)
  {
    packet |= ACTION3;
  }
  else if (key3_pressed)
  {
    packet |= ACTION4;
  }

  
  multicore_fifo_push_blocking(packet);
  irq_clear(IO_IRQ_BANK0);
}


void timer_callback()
{
  // Handle the timer interrupt
  // Check if the action was completed within the interval
  // Send the result to core 1 via the FIFO
}

void setup_timer(uint32_t interval_ms)
{
  uint32_t alarm_time = timer_hw->timerawl + interval_ms * 1000;
  irq_set_exclusive_handler(TIMER_IRQ_0, timer_callback);
  irq_set_enabled(TIMER_IRQ_0, true);
  timer_hw->alarm[0] = alarm_time;
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
inline uint32_t assemble_packet(states state, stage stage, actions action, uint8_t score, uint32_t data)
{
  uint32_t packet = 0;
  packet |= state;
  packet |= stage;
  packet |= action;
  packet |= score   * SCORE;
  packet |= data    * VALUE;
  return packet;
}


int main(void)
{
  init();

  int16_t time_rate = 0;

  uint8_t score = 0;
  int16_t time = 2000;
  actions action = ACTION1;
  stage stage = STAGE_1;
  states state = LOADING;
  

  packet = assemble_packet(state, stage, action, score, time);





  while(1){tight_loop_contents();}

  //Game Logic




  return 0;
}
