#include "p2.h"

#define SPD_CH_ID 6

/* Send Port */
QUEUING_PORT_TYPE *P3_PORT;
/* Receive Port */
SAMPLING_PORT_TYPE *P1_RECV;
SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

PD_STATUS_t *STATUS;

void p2_initialize(void);
void p2_timeTriggered(void);

void send_p3(int message) {
    send_queuing_message(P3_PORT, message);
}

int read_p1(void) {
    return read_sampling_message(P1_RECV);
}
int read_broadcast(void) {
    return read_sampling_message(P1_BROADCAST_RECV);
}

void init(void) {
    microkit_dbg_puts("Initialising P2 PPD\n");
    p2_initialize();
    STATUS->status = READY;
}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    switch (channel) {
        case SPD_CH_ID:    
            /* Run periodic application code */
            p2_timeTriggered();
            break;
        
        default:
            microkit_dbg_puts("ERROR!\n");
            break;
    }
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {};