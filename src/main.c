
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "keyboard.h"
#include "definitions.h"

#define resting_keystate 32

const int pull_pin = 16;
const int turn_pin = 17;
const int wire0_pin = 18;
const int wire1_pin = 19;

//wire position gives 1 for 1 and 0 for 0, we can add this value to its base so we don't need the define
const int wire_base = 18;

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
static bool key_press;

static alarm_id_t timer;
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
      if (action == TURN_IT)
           change_state(CORRECT);
      else if(action == YANK_IT || action == WIRE_IT) {time = 8000; change_state(INCORRECT);}
      break;
    case pull_pin:
      if(action == YANK_IT) 
           change_state(CORRECT);
      else if(action == TURN_IT || action == WIRE_IT) {time = 8000; change_state(INCORRECT);}
      break;
  case wire0_pin:
      if(action == WIRE_IT && wire_position == 0) 
           change_state(CORRECT);
      else if(action == YANK_IT || action == WIRE_IT || action == TURN_IT) {time = 8000; change_state(INCORRECT);}
      break;
  case wire1_pin:
      if(action == WIRE_IT && wire_position == 1) 
           change_state(CORRECT);
      else if(action == YANK_IT || action == WIRE_IT || action == TURN_IT) {time = 8000; change_state(INCORRECT);}
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

  
  multicore_lockout_victim_init();

  gpio_init(pull_pin);
  gpio_init(turn_pin);
  gpio_init(wire0_pin);
  gpio_init(wire1_pin);

  gpio_set_dir(pull_pin, 0);
  gpio_set_dir(turn_pin, 0);
  gpio_set_dir(wire0_pin, 0);
  gpio_set_dir(wire1_pin, 0);

  gpio_pull_up(pull_pin);
  gpio_pull_up(turn_pin);
  gpio_pull_up(wire0_pin);
  gpio_pull_up(wire1_pin);

  gpio_is_input_hysteresis_enabled(pull_pin);
  gpio_is_input_hysteresis_enabled(turn_pin);
  // gpio_is_input_hysteresis_enabled(wire0_pin);
  // gpio_is_input_hysteresis_enabled(wire1_pin);
    
    //calls the entry function for core 1
  multicore_launch_core1(core_one_main);

    //This currently goes to call a function in keyboard.c that sets up the PIO and the interrupt, but we will
    //change this depending on how we decode the keypad
  g_key = (char *)malloc(sizeof(char));
  keyboard_init(g_key, &key_press);



  key_turned    = gpio_get(turn_pin);
  wire_pulled   = gpio_get(pull_pin);

  wire_position = gpio_get(wire0_pin);



  gpio_set_irq_enabled_with_callback(pull_pin, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(pull_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);

  gpio_set_irq_enabled_with_callback(turn_pin, GPIO_IRQ_EDGE_RISE, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(turn_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);

  gpio_set_irq_enabled_with_callback(wire0_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
  gpio_set_irq_enabled_with_callback(wire1_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
}


  int64_t game_timer_callback(alarm_id_t id, void * user_data){
    multicore_fifo_push_blocking(assemble_packet(INCORRECT, NOP, score, 8000));
    callback = true;
    game_over = true;
    return 0;
    }


 int main(){
start:
  init();

  actions action = NOP;
  uint8_t selection = 0;
  uint8_t time_rate = 0;
  uint16_t start_time = 6000;
  score = 0;

  callback = false;
  game_over = false;
  key_press = false;

  char select_key = ' ';

  state = LOADING;
  time = 5000;
  do{
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));
  busy_wait_ms(5100);
  if(key_press){
    key_press = false;
    select_key = *g_key;
  }
 }while(select_key != '\n');

  select_key = ' ';
  pio_sm_restart(pio0, 0);
  state = SELECT;
  
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));
  do{
    if(key_press){
      key_press = false;
      select_key = *g_key;
      if(select_key == '<' || select_key == '>'){
        switch(select_key){
          case '<': action = TURN_IT; if(selection > 0) selection--; select_key = ' '; break;
          case '>': action = YANK_IT; if(selection < 2) selection++; select_key = ' '; break;
        }
        multicore_fifo_push_blocking(assemble_packet(SELECT, action, 0, 0));
      }
    }
  }while(select_key != '\n');

  select_key = ' ';

  selection = 0;
  switch(selection){
    case 0: time_rate = 15; break;  //3000ms to 1500ms / 100 steps is 15
    case 1: time_rate = 20; break;  //3000ms to 1000ms / 100 steps is 20
    case 2: time_rate = 22; break;  //3000ms to  800ms / 100 steps is 22
  }

 
  state = CONTINUE;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 0));
   while (select_key != '\n'){
    if(key_press){
       key_press = false;
      select_key = *g_key;
    }
   }
  select_key = ' ';


  irq_set_enabled(PIO0_IRQ_0, false);
  state = COUNTDOWN;
  time = 5000;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, time));
  busy_wait_ms(5050);


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
      if (wire_position){
          gpio_set_irq_enabled_with_callback(wire1_pin, GPIO_IRQ_EDGE_FALL, false, (void *)&action_isr);
          gpio_set_irq_enabled_with_callback(wire0_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
      }else{
          gpio_set_irq_enabled_with_callback(wire0_pin, GPIO_IRQ_EDGE_FALL, false, (void *)&action_isr);
          gpio_set_irq_enabled_with_callback(wire1_pin, GPIO_IRQ_EDGE_FALL, true, (void *)&action_isr);
      }
      
      callback = false;
      action = (actions)(((get_rand_32() % 3)  + 1) * ACTION);
      timer = add_alarm_in_ms(time + 10, game_timer_callback, NULL, false);
      multicore_fifo_push_blocking(assemble_packet(state, action, score, time));
      while(!callback){tight_loop_contents();}
      score++;
      time -= time_rate;
    }
  }while (score < 100 && !game_over);
  cancel_alarm(timer);


  irq_set_enabled(PIO0_IRQ_0, true);

  state = RESTART;
  multicore_fifo_push_blocking(assemble_packet(state, NOP, 0, 10000));
  while(select_key != '\n' || select_key != '\b'){if(key_press){key_press = false; select_key = *g_key;}}
  if(select_key == '\n') game_over = false;
  else if(select_key == '\b') game_over = true;

  if(!game_over)
    goto start;
  else
    display_exit();


  return 0;
}
