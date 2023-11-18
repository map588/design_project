#ifndef DISPLAY_FUNCTIONS
#define DISPLAY_FUNCTIONS
#include <stdint.h>

inline void countdown_bar(uint8_t index);
bool init_display();
void select_display(uint8_t key);
void countdown_bar(uint8_t index);
void populate_UI_elements(uint16_t countdown, uint8_t score);

#endif 


