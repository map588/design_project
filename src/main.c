
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "keyboard_library/keyboard.h"
#include "definitions.h"


//multipliers for adding to the FIFO packet
//  V = value, A = action, P = Points (score), S = state
//  packet =>  0xVVVVPPAS
//  Value    Score   Action   State
//[31 - 16] [15 - 8] [7 - 4] [3 - 0]
//The assemble packet function deals with this, but heres how it works

//Packet is global
uint32_t packet;

const int key0 = 16;
const int key1 = 17;
const int key2 = 18;
const int key3 = 19;

static uint8_t score = 0;

//All of these enums are defined in enum.h, in the include folder
states state;

void action_isr(void)
{
  irq_clear(IO_IRQ_BANK0);

 
  //Volatile because the value changes based on the hardware, compiler can't trust it
  volatile uint32_t irq_pin;

  // Inline assembly to read the value of R5 into irq_pin
  asm("mov %0, r5" : "=r"(irq_pin));

  actions prompt;

  //
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

  alarm_pool_create(0, 8);
  gpio_set_input_enabled(key0, 1);
  gpio_set_input_enabled(key1, 1);
  gpio_set_input_enabled(key2, 1);
  gpio_set_input_enabled(key3, 1);

  gpio_pull_down(key0);
  gpio_pull_down(key1);
  gpio_pull_down(key2);
  gpio_pull_down(key3);

    
    //calls the entry function for core 1
    multicore_launch_core1(core_one_main);

    //This currently goes to call a function in keyboard.c that sets up the PIO and the interrupt, but we will
    //change this depending on how we decode the keypad
    keyboard_init();

    //set up the GPIO interrupts
    gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
    gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
}

//entry
int main(){
  //calls the above function to initialize the clock, I/O, calls core 1's entry, and sets up the GPIO interrupts
  init();

  //dummy values that currently mean nothing, probably will be used by the game to keep track of score and time updates
  //Will probably need an array of time_rates that are for easy, medium, and hard difficulties
  int16_t time_rate = 20;
  int16_t time = 100;
  //random action selected, we will keep this part somewhere
  actions action = (actions)(((rand() % 3)  + 1) * ACTION);
  //first state is a fake loading bar
  state = LOADING;
  
  //assembles the packet to be sent to the display manager
  packet = assemble_packet(state, action, score, time);
  multicore_fifo_push_blocking(packet);


  //Does nothing forever, and interrupts will handle the rest
  while(1){tight_loop_contents();}

  //should never reach here, but we need to call this to deallocate somewhere else that will run
  display_exit();

  return 0;
}
