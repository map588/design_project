#include "display_manager.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define STAGE  0x00000010
#define SCORE  0x00000100
#define DATA   0x00010000


int key0 = 16;
int key1 = 17;
int key2 = 18;
int key3 = 19;

void action_isr(int32_t packet)
{
  multicore_fifo_push_blocking(packet);
  irq_clear_pending(TIMER_IRQ_0);
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
  irq_set_exclusive_handler(TIMER_IRQ_1, timer_callback);
  irq_set_enabled(TIMER_IRQ_1, true);
  timer_hw->alarm[0] = alarm_time;
}

int init(void)
{

  stdio_init_all();

  gpio_set_input_enabled(key0, 1);
  gpio_set_input_enabled(key1, 1);
  gpio_set_input_enabled(key2, 1);
  gpio_set_input_enabled(key3, 1);

  // gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, &action_isr);
  // gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, &action_isr);
  // gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, &action_isr);
  // gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, &action_isr);

  multicore_launch_core1(core_one_entry);

  multicore_fifo_push_blocking(0);

  if (!init_display())
  {
    exit(1);
  }
}

int main(void)
{
  init();

  int32_t packet; // {}
  int score;
  int countdown;
  int countdown_rate;
  int stage;
 

  //Game Logic






  return 0;
}
