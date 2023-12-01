
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "keyboard.h"
#include "definitions.h"

#define resting_keystate 32

const int pull_pin = 16;
const int turn_pin = 17;
const int wire1_pin = 18;
const int wire2_pin = 19;

//Assemble packet does this, but heres how it works
//  V = time, A = action, P = Points (score), S = state
//  packet =>  0xVVVVPPAS
//  time    Score   Action   State
//[31 - 16] [15 - 8] [7 - 4] [3 - 0]


//Packet is global
uint32_t packet;

static char    * g_key;
static uint8_t   score;
static states    state  = LOADING;
static actions   action = NOP;
static uint16_t  time;
static bool callback;
static bool key_turned;
static bool wire_pulled;
static bool wire_position;
static bool game_over;
static bool temp;


static alarm_id_t timer;
static alarm_id_t restart_timer;
//All of these enums are defined in enum.h, in the include folder


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
  volatile uint32_t irq_action;

  cancel_alarm(timer);
  // Inline assembly to read the time of R5 into irq_pin
  asm("mov %0, r5" : "=r"(irq_action));
  callback = true;

  switch(irq_action){
    case turn_pin:
      if(state = CONTINUE || game_over == 1){
        temp = true;
        cancel_alarm(restart_timer);
      }
      else if (action == TURN_IT)
           change_state(CORRECT);
      else change_state(INCORRECT);
      break;
    case pull_pin:
      if(action == YANK_IT) 
           change_state(CORRECT);
      else change_state(INCORRECT);
      break;
  case wire1_pin:
      if(action == WIRE_IT && wire_position == 0) 
           change_state(CORRECT);
      else change_state(INCORRECT);
      break;
  case wire2_pin:
      if(action == WIRE_IT && wire_position == 1) 
           change_state(CORRECT);
      else change_state(INCORRECT);
      break;
  default:
      change_state(INCORRECT);
      break;
  }

  irq_clear(IO_IRQ_BANK0);
}

int64_t null_call(alarm_id_t id, void * user_data){return 0;}


int init(void)
{

  if(!set_sys_clock_khz(SYS_CLK_KHZ, true))
    exit(1);

  stdio_init_all();


  timer = add_alarm_in_ms(50, null_call, NULL, false);

  if(timer == -1){
    printf("timer failed to initialize\n");
    exit(1);
  }

  cancel_alarm(timer);

  gpio_init(pull_pin);
  gpio_init(turn_pin);
  gpio_init(wire1_pin);
  gpio_init(wire2_pin);

  gpio_set_dir(pull_pin, 0);
  gpio_set_dir(turn_pin, 0);
  gpio_set_dir(wire1_pin, 0);
  gpio_set_dir(wire2_pin, 0);

  gpio_pull_up(pull_pin);
  gpio_pull_up(turn_pin);
  gpio_pull_up(wire1_pin);
  gpio_pull_up(wire2_pin);

  multicore_lockout_victim_init();
  // gpio_is_input_hysteresis_enabled(pull_pin);
  // gpio_is_input_hysteresis_enabled(turn_pin);
  // gpio_is_input_hysteresis_enabled(wire1_pin);
  // gpio_is_input_hysteresis_enabled(wire2_pin);
    
    //calls the entry function for core 1
  multicore_launch_core1(core_one_main);

    //This currently goes to call a function in keyboard.c that sets up the PIO and the interrupt, but we will
    //change this depending on how we decode the keypad
  g_key = (char *)malloc(sizeof(char));
  keyboard_init(g_key);


  key_turned    = gpio_get(turn_pin);
  wire_pulled   = gpio_get(pull_pin);
  wire_position = gpio_get(wire1_pin);



  gpio_set_irq_enabled_with_callback(pull_pin, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(pull_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);

  gpio_set_irq_enabled_with_callback(turn_pin, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(turn_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);

  gpio_set_irq_enabled_with_callback(wire1_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(wire2_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
}


  int64_t game_timer_callback(alarm_id_t id, void * user_data){
    multicore_fifo_push_blocking(assemble_packet(INCORRECT, NOP, score, time));
    busy_wait_ms(2000);
    callback = true;
    game_over = true;
    return 0;
    }

  int64_t restart_timer_callback(alarm_id_t id, void * user_data){
    game_over = true;
    callback = true;
    return 0;
  }

  int main(){
 start:
   init();
   actions action = NOP;
   uint8_t selection = 0;
   uint8_t time_rate = 0;
   uint16_t start_time = 3000;
   score = 0;

   callback = false;
   game_over = false;
   temp = false;

   state = LOADING;
   time = 20000;
   multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));
   busy_wait_ms(1500);

   state = SELECT;
   while (*g_key == resting_keystate){tight_loop_contents();}
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

   //selection = 0;
   switch (selection)
   {
   case 0:
     time_rate = 15;
     break; // 3000ms to 1500ms / 100 steps is 15
   case 1:
     time_rate = 20;
     break; // 3000ms to 1000ms / 100 steps is 20
   case 2:
     time_rate = 22;
     break; // 3000ms to  800ms / 100 steps is 22
   }

   state = CONTINUE;
   multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));
   while (*g_key != '\n'){tight_loop_contents();}
   //while (!temp){tight_loop_contents();}


  state = COUNTDOWN;
  time = 3000;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));
  busy_wait_ms(3150);


  state = GAME;
  time = (uint16_t) start_time;

  do{
    if(score != 0 && score % 20 == 0) {
      state = RANDOM_KEY;
      timer = add_alarm_in_ms(3010, game_timer_callback, NULL, false);
      multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, (rand()%1000)));
      while(!callback){tight_loop_contents();}
  }
    else{
      callback = false;
      action = (actions)(((rand() % 3)  + 1) * ACTION);
      timer = add_alarm_in_ms(time + 10, game_timer_callback, NULL, true);
      multicore_fifo_push_blocking(assemble_packet(state, action, score, time));
      while(!callback){tight_loop_contents();}
      score++;
      time -= time_rate;
    }
  }while (score < 100 && !game_over);
  

  state = RESTART;
  game_over = false;
  restart_timer = add_alarm_in_ms(10010, restart_timer_callback, NULL, false);
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 10000));

 do{
   if(*g_key == '\n'){
     cancel_alarm(restart_timer);
     printf("Restarting\n");
      goto start;
   }
 }while(!game_over);

  busy_wait_ms(1000);
  display_exit();
  return 0;
}
