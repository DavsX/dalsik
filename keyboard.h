#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "dalsik.h"

#define DEBOUNCE_MAX 0x02
#define DEBOUNCE_LOW 0x00
#define DEBOUNCE_CHANGING 0xFF

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02

typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} ChangedKeyCoords;

class Keyboard {
    private:
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
    public:
        Keyboard();
        ChangedKeyCoords matrix_scan();
};

#endif
