#ifndef MAIN_H
#define MAIN_H

#include "display_manager.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

//multipliers for adding to the FIFO packet
//  V = value, A = action (enum), P = Points, I = index, S = state (enum)
//  packet =>  0xVVVAPPIS
#define INDEX  0x00000010
#define SCORE  0x00000100
#define VALUE  0x00100000
#define EDGE_LOW_MASK  0x4444
#define EDGE_HIGH_MASK 0x8888


// TODO: Decide if we want to store more values as globals and assemble them in the irq handler
states state;
bool user_input = false;

int key0 = 16;
int key1 = 17;
int key2 = 18;
int key3 = 19;


#endif