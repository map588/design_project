
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"

#include "display_manager.h"

void core_one_interrupt_handler(void) {
while (multicore_fifo_rvalid())
    {
        uint32_t data =  multicore_fifo_pop_blocking();
        uint8_t state =  data >> 28;
        uint8_t score = (data >> 20) & 0xFF;
        uint8_t stage = (data >> 16) & 0x0F;
        uint8_t time  =  data        & 0xFF;
    //...

        switch(state){
            case INIT:
                init_game_disp();
                break;
            case SELECT:
                selection_disp(data & 0x0F);
                break;
            case LOADING:
                loading_disp((data & 0x0F) / 10.0);
                break;
            case GAME:
                game_disp(score, stage, time);
                break;
            case CORRECT:
                correct_disp();
                break;
            case INCORRECT:
                incorrect_disp();
                break;
            case RESTART:
                restart_disp();
                break;
            default:
                break;

        }
    }
    multicore_fifo_clear_irq();
}

void core_one_entry(void) {
    
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1 , core_one_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

  while (1) {
    tight_loop_contents();
  }
}

