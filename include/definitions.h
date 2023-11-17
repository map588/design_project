#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define VALUE   0x00010000
#define SCORES  0x00000100
#define ACTION  0x00000010

#include "../lib/GUI/GUI_Paint.h"
#include "../lib/LCD/LCD_2in.h"
#include "enums.h"
#include <stdint.h>


void keyboard_init();
inline uint32_t assemble_packet(states state, actions action, uint8_t score, uint16_t data)
 {
    uint32_t packet = 0;
    packet |= state;
    packet |= action;
    packet |= score * SCORES;
    packet |= data * VALUE;
    return packet;
 }

#endif