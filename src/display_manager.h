#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "GUI_Paint.h"

    UWORD IMAGE_SIZE = 320 * 240 * 2;

    /// @brief allocate all images into memory

    static UWORD WELCOME   [IMAGE_SIZE];
    static UWORD LOADING   [IMAGE_SIZE];
    static UWORD MENU      [IMAGE_SIZE];
    static UWORD UI        [IMAGE_SIZE];
    static UWORD GAME_OVER [IMAGE_SIZE];
    static UWORD WIN       [IMAGE_SIZE];

    static UWORD *current_image;


    void init_display(void);

#endif