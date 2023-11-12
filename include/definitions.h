#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define INDEX   0x00000010
#define SCORES  0x00000100
#define VALUE   0x00100000
#define ACTION  0x00010000

#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "enums.h"
#include <stdint.h>


void keyboard_init();
inline uint32_t assemble_packet(states state, uint8_t index, actions action, uint8_t score, uint16_t data)
 {
    uint32_t packet = 0;
    packet |= state;
    packet |= action;
    packet |= index * INDEX;
    packet |= score * SCORES;
    packet |= data * VALUE;
    return packet;
 }

#endif