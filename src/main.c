
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
//  V = time, A = action, P = Points (score), S = state
//  packet =>  0xVVVVPPAS
//  time    Score   Action   State
//[31 - 16] [15 - 8] [7 - 4] [3 - 0]


//Packet is global
uint32_t packet;

static char    * g_key;
static uint8_t   score  = 0;
static states    state  = LOADING;
static actions   action = NOP;
static uint16_t  time  = 0;

static alarm_pool_t *core0_pool;
static alarm_id_t timer;
//All of these enums are defined in enum.h, in the include folder

inline static void update_time(uint16_t new_time){
  time = new_time;

  uint32_t packet_t = 0;
  packet_t |= time * TIME;
  packet_t |= state;
  packet_t |= score * SCORES;
  packet_t |= action;

  multicore_fifo_push_blocking(packet_t);
}

inline static void change_state(states new_state){
  state =  new_state;

  uint32_t packet_t = 0;
  packet_t |= time * TIME;
  packet_t |= state;
  packet_t |= score * SCORES;
  packet_t |= action;

  multicore_fifo_push_blocking(packet_t);
}
void action_isr(void){
  volatile uint32_t irq_pin;

  // Inline assembly to read the time of R5 into irq_pin
  asm("mov %0, r5" : "=r"(irq_pin));

  actions prompt;


  //Right now this is being determined GPIO alone, just to test the display manager, but later the game logic will determine the state,
  //and this interrupt will check which pin is pressed, and what state the game is in, to determine what to send
  switch (irq_pin)
  {
    case key0:
        packet = assemble_packet(LOADING, 0, 0, 2000);
        break;

    case key1:
        prompt = (actions)(((get_rand_32() % 3)  + 1) * ACTION);
        packet = assemble_packet(GAME, prompt, score, 3000);
        score++;
        break;

    case key2:
        packet = assemble_packet(SELECT, TURN_IT, 0, 0);
        break;

    case key3: 
        packet = assemble_packet(SELECT, YANK_IT, 0, 0);
        break;

    default:
        packet = assemble_packet(LOADING, 0, 0, 200);
    break;
  }

  multicore_fifo_push_blocking(packet);
  irq_clear(IO_IRQ_BANK0);
}

int64_t null_callback(alarm_id_t id, void * user_data){return 0;}


int init(void)
{

  if(!set_sys_clock_khz(SYS_CLK_KHZ, true))
    exit(1);

  stdio_init_all();

  alarm_pool_create(0, 4);

  timer = alarm_pool_add_alarm_in_ms(core0_pool, 50, null_callback, NULL, true);

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

  gpio_is_input_hysteresis_enabled(key0);
  gpio_is_input_hysteresis_enabled(key1);
  gpio_is_input_hysteresis_enabled(key2);
  gpio_is_input_hysteresis_enabled(key3);

    
    //calls the entry function for core 1
  multicore_launch_core1(core_one_main);

    //This currently goes to call a function in keyboard.c that sets up the PIO and the interrupt, but we will
    //change this depending on how we decode the keypad
  char *g_key = (char *)malloc(sizeof(char));
  keyboard_init(g_key);


    //TODO: set up the timer interrupt
    //TODO: change the gpio interrupts to PIO interrupts that simplify our inputs
    //Inputs are a the toggle cord, the rising or falling edge of the turn key, and the wire terminals




  gpio_set_irq_enabled_with_callback(key0, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(key1, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(key2, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(key3, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
}


 int main(){

  init();

  uint8_t selection = 0;
  uint8_t time_rate = 0;

  state = LOADING;
  time = 2000;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));

  // state = INTRO;
  // while (*g_key != '\n'){tight_loop_contents();}
  // multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));


  state = SELECT;
  while (*g_key == NULL){tight_loop_contents();}
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));
  do{
    if(*g_key == '<' || *g_key == '>'){
      switch(*g_key){
        case '<': action = (actions)0x10; if(selection <= 0) selection--; break;
        case '>': action = (actions)0x20; if(selection >= 2) selection++; break;
      }
     multicore_fifo_push_blocking(assemble_packet(SELECT, action, 0, 0));
    }

  }while(*g_key != '\n');

  switch(selection){
    case 0: time_rate = 15; break;  //3000ms to 1500ms / 100 steps is 15
    case 1: time_rate = 20; break;  //3000ms to 1000ms / 100 steps is 20
    case 2: time_rate = 22; break;  //3000ms to  800ms / 100 steps is 22
  }

 

  // TODO impliment a START state, and screen
  state = CONTINUE;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));
  while (*g_key != '\n'){tight_loop_contents();}

  state = COUNTDOWN;
  time = 4000;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));
  busy_wait_ms(time + 100);

  //TODO STAGE 3: Display the game screen, start game loop
  
  //TODO buttons need debounced, and the random 3 digit number needs to be generated when score % 20 = 0
  
  //if(score % 20 == 0) { state = KEYENTRY; multicore_fifo_push_blocking(assemble_packet(state, NOP, (rand()%1000), 0)); }

  //action = (actions)(((rand() % 3)  + 1) * ACTION);

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
