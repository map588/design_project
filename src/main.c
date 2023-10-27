#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"

#include "display_manager.h"
#include "core_one.h"

int main(void)
{
    stdio_init_all();
    multicore_launch_core1(core_one_entry);

   if(!init_display()){
        exit()
    }   

    
    return 0;
}
