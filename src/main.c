
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"

#include "keyboard_library/keyboard.h"
#include "definitions.h"

const int key0 = 16;
const int key1 = 17;
const int key2 = 18;
const int key3 = 19;

//Assemble packet does this, but heres how it works
//  V = value, A = action, P = Points (score), S = state
//  packet =>  0xVVVVPPAS
//  Value    Score   Action   State
//[31 - 16] [15 - 8] [7 - 4] [3 - 0]


//Packet is global
uint32_t packet;


static uint8_t   score  = 0;
static states    state  = LOADING;
static actions   action = NOP;
static uint16_t  value  = 0;
//All of these enums are defined in enum.h, in the include folder

inline void update_time(uint16_t time){
  uint32_t packet_t = 0;

  packet_t |= time * VALUE;
  packet_t |= state;
  packet_t |= score * SCORES;
  packet_t |= action;

  multicore_fifo_push_blocking(packet_t);
}
inline void change_state(states new_state){
  state = new_state;
  uint32_t packet_t = 0;
  packet_t |= value * VALUE;
  packet_t |= state;
  packet_t |= score * SCORES;
  packet_t |= action * ACTION;

  multicore_fifo_push_blocking(packet_t);
}
void action_isr(void){
  //Volatile because the value changes based on the hardware, compiler can't trust it
  volatile uint32_t irq_pin;

  // Inline assembly to read the value of R5 into irq_pin
  asm("mov %0, r5" : "=r"(irq_pin));

  actions prompt;

  //depending on the key pressed, we will send a different packet to core 1

  //Right now this is being determined GPIO alone, just to test the display manager, but later the game logic will determine the state,
  //and this interrupt will check which pin is pressed, and what state the game is in, to determine what to send
  switch (irq_pin)
  {
    case key0:
        packet = assemble_packet(LOADING, 0, 0, 2000);
        break;

    case key1:
        prompt = (actions)(((get_rand_32() % 3)  + 1) * ACTION);  //This will be used elsewhere when we impliment the game logic
        packet = assemble_packet(GAME, prompt, score, 3000);
        score++;
        break;

    case key2:
        packet = assemble_packet(TESTING, NOP, 0, 0);
        break;

    case key3: 
        packet = assemble_packet(SELECT, YANK_IT, 0, 0);
        break;

    default:
        packet = assemble_packet(LOADING, 0, 0, 200);
    break;
  }

  //sends the packet to the display manager using the FIFO
  multicore_fifo_push_blocking(packet);
  irq_clear(IO_IRQ_BANK0);
}


int init(void)
{

  if(!set_sys_clock_khz(SYS_CLK_KHZ, true))
    exit(1);

  stdio_init_all();

  alarm_pool_create(0, 4);
  gpio_init(key0);
  gpio_init(key1);
  gpio_init(key2);
  gpio_init(key3);

  gpio_set_dir(key0, 0);
  gpio_set_dir(key1, 0);
  gpio_set_dir(key2, 0);
  gpio_set_dir(key3, 0);

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
  int16_t time = 5000;

  action = (actions)(((rand() % 3)  + 1) * ACTION);
  state  = LOADING;
  
  //assembles the packet to be sent to the display manager
  packet = assemble_packet(state, action, score, time);

  multicore_fifo_push_blocking(packet);


  //TODO STAGE 1: Loading, then display select screen
  //TODO STAGE 2: Display the start screen, wait for input
  //TODO STAGE 3: Display the game screen, start game loop
  //TODO STAGE 4: Main game loop: chose random action, start timer interrupt, wait for input -> Timer interrupt triggers fail callback when reached
  //TODO STAGE 5: Continue this loop until a new round is reached
  //TODO STAGE 6: Go back to main game loop
  //TODO STAGE 7: After 100 successful moves, display win screen, if the player fails at some point, display lose screen
  //TODO STAGE 8: Wait for input, if the player presses the start button, go back to stage 1, otherwise, do nothing




  //Does nothing forever, and interrupts will handle the rest
  while(1){tight_loop_contents();}

  //should never reach here, but we need to call this to deallocate somewhere else that will run
  display_exit();

  return 0;
}
