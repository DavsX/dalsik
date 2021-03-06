#ifndef SLAVE_REPORT_H
#define SLAVE_REPORT_H

#include "matrix.h"

namespace SlaveReport {
    void send_changed_key(ChangedKeyCoords coords);
    uint8_t encode_slave_report_data(ChangedKeyCoords coords);
    ChangedKeyCoords decode_slave_report_data(uint8_t data);
}

#endif
