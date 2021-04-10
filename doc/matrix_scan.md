# Matrix scanning

Modules:

* [matrix.h](https://github.com/DavsX/dalsik/blob/master/matrix.h)
* [matrix.ino](https://github.com/DavsX/dalsik/blob/master/matrix.ino)



In the [Keyboard wiring](keyboard_wiring.md) document we saw how each key is wired up to the microcontroller. Scanning the keyboard matrix means checking the state (pressed or released) of each key and finding those, whose state has changes.

For tracking the state of each key I am using an array `uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_PIN_COUNT]` in [matrix.h](https://github.com/DavsX/dalsik/blob/master/matrix.h). The value of `0` means, that a key is released, while `1` is for pressed keys.

## Pin initialization

The pin-to-row mapping is defined in [dalsik.h](https://github.com/DavsX/dalsik/blob/master/dalsik.h) like this:

```c++
// dalsik.h
const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};
```

Each pin is initialized in the `Matrix::Matrix()` constructor with `PinUtils::pinmode_input_pullup` using two for loops.

## Scanning

<img src="keyboard_matrix.png" alt="keyboard_matrix" style="zoom: 50%;" />

Scanning is implemented in `Matrix::scan()`. In it we loop through each row and we check each column. The pins are all set to the _high_ signal (pull-up) - so reading from each column pin in idle mode returns `1`. 

We set each row ping to ground/`0` (`PinUtils::pinmode_output_low`) and for each row we read each column pin one by one.

* If the key for that row and column is not pressed, then we read a high signal for that pin (because of the pull-up resistor).
* If the key for that row and columns is pressed, then the pull-up high signal of the column's pin is connected to the ground (to the row's pin) and because of that we read a low signal.

After checking each column we set the row's pin back to `pinmode_input_pullup` and proceed to the next row. The `Matrix::scan()` function returns the first detected key change via the `ChangedKeyCoords` struct:

```c++
// matrix.h
typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} ChangedKeyCoords;
```

The type of the change can be:

```c++
// matrix.h
#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02
```

The column's state is read by inverting the signal on the pin:

```c++
// matrix.ino
uint8_t input = !PinUtils::read_pin(COL_PINS[col]);
uint8_t debounced_input = this->debounce_input(row, col, input);

if (debounced_input == DEBOUNCE_CHANGING) {
    continue; // Wait, till the value stabilizes
}

if (debounced_input != this->keystate[row][col]) {
    this->keystate[row][col] = debounced_input;
    PinUtils::pinmode_input_pullup(ROW_PINS[row]);

    if (debounced_input == DEBOUNCE_MAX) {
        return ChangedKeyCoords { EVENT_KEY_PRESS, row, col };
    } else {
        return ChangedKeyCoords { EVENT_KEY_RELEASE, row, col };
    }
}
```

After that we check the previous state of the key and if it's different, then we update the keystate to the current value and return a `ChangedKeyCoords` struct.

## Debouncing

<img src="contact_bouncing.png" alt="contact_bouncing" style="zoom: 50%;" />

When we press a key, the two metal electrodes don't instantly form a connection, but they bounce a bit (between connected and disconnected state). Without debouncing we could interpret a single key press as several press and release events, which is bad for a keyboard (If I press a button once, I expect the keyboard to send a single press event).

There are multiple ways of debouncing an input. One could for example implement it using `read -> sleep -> read` (and comparing the two values before and after). In Dalsik I use a separate