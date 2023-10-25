#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"

#include "display_manager.h"

int main(void)
{
    stdio_init_all();

    
   if(init_display()){
    start_game_display();
    }   

    
    return 0;
}
