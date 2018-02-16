#include <util/atomic.h>

RingBuffer::RingBuffer() {
    for (uint8_t i = 0; i < BUFFER_LENGTH; i++) {
        this->buffer[i] = 0x00;
    }
    this->size = 0;
    this->read_index = 0;
    this->write_index = 0;
}

void RingBuffer::append_elem(uint8_t elem) {
    if (elem == 0x00 || this->size >= BUFFER_LENGTH) {
        return;
    }
    this->buffer[ this->write_index ] = elem;
    this->write_index = (this->write_index + 1) % BUFFER_LENGTH;
    this->size++;
}

uint8_t RingBuffer::get_next_elem() {
    if (this->size == 0) {
        return 0x00;
    }

    uint8_t elem = 0x00;

    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        elem = this->buffer[ this->read_index ];
        this->buffer[ this->read_index ] = 0x00;

        this->read_index = (this->read_index + 1) % BUFFER_LENGTH;
        this->size--;
    }

    return elem;
}

uint8_t RingBuffer::has_data() {
    return this->size;
}
