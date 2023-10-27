// Description: This file contains the code for the second core of the RP2040. 
// This core is responsible for displaying the game on the screen. 
// It receives data from the first core and displays the appropriate screen.

#include "core_one.h"

void core_one_interrupt_handler(void) {
while (multicore_fifo_rvalid())
    {
        uint32_t data =  multicore_fifo_pop_blocking();

        uint16_t value =  data >> 16 & 0xFFFF;      // 16 MSBs for data, time remaining
         uint8_t score = (data >> 8) &   0xFF;      // 0 - 255 , score should only reach 100
         uint8_t stage = data        &   0xF0;      // 0 - 15, stage should only reach 5, used for L, R, and ENTER in selection
         uint8_t state = data        &   0x0F;      // 0 - 15, INIT, SELECT, LOADING, GAME, CORRECT, INCORRECT, RESTART

        switch(state){
            case INIT:
                init_game_disp();
                break;
            case SELECT:
                selection_disp(stage);
                break;
            case LOADING:
                loading_disp((data & 0x0F) / 10.0);
                break;
            case GAME:
                game_disp(score, stage, value);
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

