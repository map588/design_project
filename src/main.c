
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "display_manager.h"
#include "keyboard_library/keyboard.h"


//multipliers for adding to the FIFO packet
//  V = value, A = action (enum), P = Points, I = index, S = state (enum)
//  packet =>  0xVVVAPPIS


// TODO: Decide if we want to store more values as globals and assemble them in the irq handler

void main_disp(uint16_t countdown, uint8_t score, actions prompt);
void loading_disp(uint16_t countdown, uint8_t iterations);

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

  irq_clear(SIO_IRQ_PROC1);
  irq_set_enabled(SIO_IRQ_PROC1, false);
  

  volatile uint32_t irq_pin;
  // Inline assembly to read the value of R0 into irq_pin
  asm("mov %0, r5" : "=r"(irq_pin));

  
  actions prompt;

  switch (irq_pin)
  {
    case key0:
      packet = assemble_packet(LOADING, 0, 0, 0, 500);
      break; 
    case key1:
      prompt = (actions)(((get_rand_32() % 3)  + 1) * ACTION);  //This will be used elsewhere when we impliment the game logic
      packet = assemble_packet(GAME, 0, prompt, score, 1000);
      score++;
      break;
    case key2:
      packet = assemble_packet(SELECT,0,0,0,0);
      break;
    case key3: 
      packet = assemble_packet(SELECT,1,0,0,0);
      break;
    default:
    packet = assemble_packet(LOADING, 1, 0, 0, 200);
    break;
  }
    irq_set_enabled(SIO_IRQ_PROC1, true);
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
  
  gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);
  gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, (void *) &action_isr);

  keyboard_init();
  multicore_launch_core1(core_one_entry);

}


int main(void)
{
  init();

  int16_t time_rate = 20;
  int16_t time = 100;
  actions action = (actions)(((rand() % 3)  + 1) * ACTION);

  state = LOADING;
  
  packet = assemble_packet(state, 0, action, score, time);
  multicore_fifo_push_blocking(packet);

  // do{
  //   //Wait for select


  // }while(1);
  
  //500ms load

  //Game Logic

  //Press Enter to start

  // do{


  // }while(1);

  //generate random number and pass to core 1
  //wait for response
  //if correct, increment score and time rate
  //if incorrect, decrement score and time rate


  
  while(1){tight_loop_contents();}


  display_exit();

  return 0;
}
