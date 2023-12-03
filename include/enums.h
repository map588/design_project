#ifndef ENUMS_H
#define ENUMS_H

typedef enum // Lookup for the state of the game
{
    LOADING,
    SELECT,
    CONTINUE,
    COUNTDOWN,
    GAME,
    KEYPRESS,
    CORRECT,
    INCORRECT,
    RESTART,
    RANDOM_KEY
} states;


typedef enum //Lookup for the action of the game
{TURN_IT = 0x10, YANK_IT = 0x20, WIRE_IT = 0x30, NOP = 0x40} actions;

#endif //ENUMS_H