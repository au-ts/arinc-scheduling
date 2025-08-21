#include "interpartitioncomm.h"
#include "partition.h"
#include "upd_types.h"
#include "port.h"
#include <libmicrokitco.h>

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

int receive_queuing_message(QUEUING_PORT_TYPE *port, QUEUE_MSG *output_buf, process_internal *caller, int num_retries) {
    if (output_buf == NULL) {
        printf("ERROR: invalid output_buf!\n");
        return ERR_BUF_INVALID;
    }
    /* Retry if empty (num_retries should be 0 for pCo) */
    while (num_retries > 0) {
        if (port->size == 0) {
            --num_retries;
            microkit_cothread_yieldto(ROOT_COTHREAD_REF);
        } else {
            break;
        }
    }
    /* Exit on empty buffer and no retries remaining */
    if (num_retries == 0 && port->size == 0) {
        return ERR_BUF_EMPTY;
    }
    /* Start Critical Section */
    caller->critical_section = 1;
    QUEUE_MSG msg = port->buffer[port->head];
    if (msg.status == INVALID) {
        return ERR_MSG_INVALID;
    }
    port->buffer[port->head].status = INVALID;
    port->buffer[port->head].data = 0;
    port->head = (port->head + 1) % port->max_len;
    --port->size;
    *output_buf = msg;
    caller->critical_section = 0;
    /* End Critical Section */
    /* Need to check flag for preempted and yield if I am aCo */
    /* This should only be set if caller was aCo */
    if (caller->preempted) {
        caller->preempted = 0;
        microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    }
    return 0;
}

void transfer_queuing_buffers(QUEUING_PORT_TYPE *from, QUEUING_PORT_TYPE *to) {
    while (from->size) {
        /* Only transfer if the receiving port is not full */
        if (to->size < to->max_len) {
            QUEUE_MSG msg = from->buffer[from->head];
            if (msg.status == VALID) {
                if (send_queuing_message(to, msg.data) != 0) {
                    microkit_dbg_puts("Error queuing to port\n");
                    break;
                }
                from->buffer[from->head].status = INVALID;
                from->buffer[from->head].data = 0;
                from->head = (from->head + 1) % from->max_len;
                --from->size;
            } else {
                break;
            }
        } else {
            /* Stop if receiving port full */
            break;
        }
    }
}
