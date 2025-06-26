#include "interpartitioncomm.h"
#include "port.h"

void check_set_message(SAMPLING_PORT_TYPE *from, SAMPLING_PORT_TYPE *to) {
    if (from->status == VALID && from->empty == FULL) {
        to->message = from->message;
        to->empty = FULL;
        to->status = VALID;
    }
}

void reset_sampling_port(SAMPLING_PORT_TYPE *port) {
    port->empty = EMPTY;
    port->status = INVALID;
}

void init_queuing_port(QUEUING_PORT_TYPE *port) {
    port->head = 0;
    port->size = 0;
    port->max_len = QUEUING_BUFFER_LEN;

    for (int i = 0; i < QUEUING_BUFFER_LEN; ++i) {
        port->buffer[i].data=0;
        port->buffer[i].status = 0;
    }
}

void write_sampling_message(SAMPLING_PORT_TYPE *port, int message) {
    (port->message) = message;
    port->empty = FULL;
    port->status = VALID;
}

int read_sampling_message(SAMPLING_PORT_TYPE *port) {
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

int send_queuing_message(QUEUING_PORT_TYPE *port, int message) {
    /* Exit on full buffer */
    if (port->size == port->max_len) {
        return ERR_BUF_FULL;
    };
    int tail = (port->head + port->size) % port->max_len;
    port->buffer[tail].data = message;
    port->buffer[tail].status = VALID;
    ++port->size;
    return 0;
}

int receive_queuing_message(QUEUING_PORT_TYPE *port) {
    /* Exit on empty buffer */
    if (port->size == 0) {
        return ERR_BUF_EMPTY;
    }
    QUEUE_MSG msg = port->buffer[port->head];
    if (msg.status == INVALID) {
        return ERR_MSG_INVALID;
    }
    port->buffer[port->head].status = INVALID;
    port->buffer[port->head].data = 0;
    port->head = (port->head + 1) % port->max_len;
    --port->size;

    return msg.data;
}

void transfer_queuing_buffers(QUEUING_PORT_TYPE *from, QUEUING_PORT_TYPE *to) {
    while (from->size) {
        /* Only transfer if the receiving port is not full */
        if (to->size < to->max_len) {
            int msg = receive_queuing_message(from);

            /* Not invalid message (non-empty known from loop cond.)*/
            if (msg != ERR_MSG_INVALID) {
                if (send_queuing_message(to, msg) != 0) {
                    microkit_dbg_puts("Error queuing to port\n");
                    break;
                }

            }
            else {
                /* End transfer as we want to maintain order of messages */
                break;
            }
        } else {
            /* Stop if receiving port full */
            break;
        }    
    } 
}
