#ifndef CORE_ONE_H
#define CORE_ONE_H

#include "display_manager.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

void core_one_interrupt_handler(void);
void core_one_entry(void);

#endif // CORE_ONE_H