#include "interpartitioncomm.h"
#include "port.h"

void check_set_message(volatile SAMPLING_PORT_TYPE *from, volatile SAMPLING_PORT_TYPE *to) {
    if (from->status == VALID && from->empty == FULL) {
        to->message = from->message;
        to->empty = FULL;
        to->status = VALID;
    }
}

void reset_port(volatile SAMPLING_PORT_TYPE *port) {
    port->empty = EMPTY;
    port->status = INVALID;
}

void write_to_port(volatile SAMPLING_PORT_TYPE *port, int message) {
    (port->message) = message;
    port->empty = FULL;
    port->status = VALID;
}

int read_from_port(volatile SAMPLING_PORT_TYPE *port) {
    if (port->empty == FULL && port->status == VALID) {
        if (port->message >= 0) {
            return (port->message);
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}