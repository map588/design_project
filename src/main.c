
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "display_manager.h"
#include "keyboard_library/keyboard.h"


//multipliers for adding to the FIFO packet
//  V = value, A = action, P = Points (score), S = state
//  packet =>  0xVVVVPPAS

// TODO: Decide if we want to store more values as globals and assemble them in the irq handler

    uint32_t packet;

const int key0 = 16;
const int key1 = 17;
const int key2 = 18;
const int key3 = 19;

static uint8_t score = 0;
states state;

void action_isr(void)
{
  irq_clear(IO_IRQ_BANK0);

 

  volatile uint32_t irq_pin;
  // Inline assembly to read the value of R0 into irq_pin
  asm("mov %0, r5" : "=r"(irq_pin));

  
  actions prompt;

  switch (irq_pin)
  {
    case key0:
      packet = assemble_packet(LOADING, 0, 0, 500);
      break; 
    case key1:
      prompt = (actions)(((get_rand_32() % 3)  + 1) * ACTION);  //This will be used elsewhere when we impliment the game logic
      packet = assemble_packet(GAME, prompt, score, 1000);
      score++;
      break;
    case key2:
      packet = assemble_packet(SELECT, 0, 0, 0);
      break;
    case key3: 
      packet = assemble_packet(SELECT, 1, 0, 0);
      break;
    default:
    packet = assemble_packet(LOADING, 0, 0, 200);
    break;
  }
    multicore_fifo_push_blocking(packet);
}


int init(void)
{

  if(!set_sys_clock_khz(SYS_CLK_KHZ, true))
  {
    exit(1);
  }

  stdio_init_all();


  gpio_set_input_enabled(key0, 1);
  gpio_set_input_enabled(key1, 1);
  gpio_set_input_enabled(key2, 1);
  gpio_set_input_enabled(key3, 1);

  gpio_pull_down(key0);
  gpio_pull_down(key1);
  gpio_pull_down(key2);
  gpio_pull_down(key3);

    
    multicore_launch_core1(core_one_main);
    keyboard_init();

    gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
}


int main(){
  init();

  int16_t time_rate = 20;
  int16_t time = 100;
  actions action = (actions)(((rand() % 3)  + 1) * ACTION);

  state = LOADING;
  
  packet = assemble_packet(state, action, score, time);
  multicore_fifo_push_blocking(packet);


  
  while(1){tight_loop_contents();}


  display_exit();

  return 0;
}
