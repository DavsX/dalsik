#include "dalsik.h"
#include "keymap.h"

#define BUFFER_SIZE 64

static const char PREFIX[] = { 'D', 'A', 'V', 'S' };
static const char CMD_SET_KEY_PREFIX[] = { 'S', 'E', 'T', '_', 'K', 'E', 'Y' };
static const char CMD_GET_KEY_PREFIX[] = { 'G', 'E', 'T', '_', 'K', 'E', 'Y' };
static const char CMD_CLEAR_KEYMAP[] = { 'C', 'L', 'E', 'A', 'R', '_', 'K', 'E', 'Y', 'M', 'A', 'P' };
static const char CMD_PING[] = { 'P', 'I', 'N', 'G' };

char serial_buffer[BUFFER_SIZE] = {0};
uint8_t serial_index = 0;

void process_serial_command(Keyboard* keyboard)
{
    while (Serial.available()) {
        char c = (char) Serial.read();

        if (c == '\n') {
            if (memcmp(serial_buffer, PREFIX, sizeof(PREFIX)) == 0) {
                uint8_t res = execute_command(keyboard);
                if (res) {
                    Serial.print("CMD_ERROR ");
                    Serial.print(res);
                    Serial.print("\n");
                } else {
                    Serial.println("CMD_OK");
                }
            } else {
                Serial.println("CMD_PARSE_ERROR");
            }
            clear_buffer();
        } else {
            if (serial_index > BUFFER_SIZE) {
                clear_buffer();
                return;
            } else {
                serial_buffer[serial_index++] = c;
            }
        }
    }
}

uint8_t execute_command(Keyboard* keyboard)
{
    char* buffer = &(serial_buffer[sizeof(PREFIX)]);

#if DEBUG
    Serial.print("BUFFER|");
    Serial.print(serial_buffer);
    Serial.print("|");
    Serial.print(buffer);
    Serial.print("|\n");
    Serial.println(serial_index);
#endif


    if (memcmp(buffer, CMD_SET_KEY_PREFIX, sizeof(CMD_SET_KEY_PREFIX)) == 0) {
        if (sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX) + 6 > serial_index) {
            return 2; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX)]);
        uint8_t layer  = buffer[0x00];
        uint8_t row    = buffer[0x01];
        uint8_t col    = buffer[0x02];
        uint8_t key_b1 = buffer[0x03];
        uint8_t key_b2 = buffer[0x04];
        uint8_t key_b3 = buffer[0x05];

#if DEBUG
        Serial.print("|");
        for (int i = 0; i < 6; i++) {
            Serial.print(buffer[i], HEX);
        }
        Serial.print("|\n");
#endif

        if (layer >= LAYER_COUNT) return 3; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 4; // Invalid row
        if (col >= COL_PIN_COUNT) return 5; // Invalid col

        KeyInfo key = { key_b1, key_b2, key_b3 };
        keyboard->keyreport.keymap.set_key(layer, row, col, key);

        return 0;
    } else if (memcmp(buffer, CMD_GET_KEY_PREFIX, sizeof(CMD_GET_KEY_PREFIX)) == 0) {
        if (sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX) + 3 > serial_index) {
            return 6; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX)]);
        uint8_t layer  = buffer[0x00];
        uint8_t row    = buffer[0x01];
        uint8_t col    = buffer[0x02];

#if DEBUG
        Serial.print("|");
        for (int i = 0; i < 3; i++) {
            Serial.print(buffer[i], HEX);
        }
        Serial.print("|\n");
#endif

        if (layer >= LAYER_COUNT) return 7; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 8; // Invalid row
        if (col >= COL_PIN_COUNT) return 9; // Invalid col

        KeyInfo key = keyboard->keyreport.keymap.get_key_from_layer(layer, row, col);

        Serial.print("KEY|");
        Serial.print(key.type);
        Serial.print("|");
        Serial.print(key.generic_bytes.byte1);
        Serial.print("|");
        Serial.print(key.generic_bytes.byte2);
        Serial.print("|\n");

        return 0;
    } else if (memcmp(buffer, CMD_CLEAR_KEYMAP, sizeof(CMD_CLEAR_KEYMAP)) == 0) {
        Serial.println("Clearing keymap");
        keyboard->keyreport.keymap.clear();
        Serial.println("Keymap cleared");
        return 0;
    } else if (memcmp(buffer, CMD_PING, sizeof(CMD_PING)) == 0) {
        Serial.println("PONG");
        return 0;
    }

    return 1;
}

void clear_buffer()
{
    memset(&serial_buffer, 0, sizeof(char)*BUFFER_SIZE);
    serial_index = 0;
}
