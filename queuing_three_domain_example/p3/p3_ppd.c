#include "p3.h"

#define SPD_CH_ID 7
PD_STATUS_t *STATUS;

/* RECEIVE PORT */
QUEUING_PORT_TYPE *P2_RECV;
SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

void p3_initialize(void);
void p3_timeTriggered(void);

int read_p2(void) {
    return receive_queuing_message(P2_RECV);
}

int read_broadcast(void) {
    return read_sampling_message(P1_BROADCAST_RECV);
}

void init(void) {
    microkit_dbg_puts("Initialising P3 PPD\n");
    p3_initialize();
    STATUS->status = READY;
}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    switch (channel) {
        case SPD_CH_ID:    
            /* Run periodic application code */
            p3_timeTriggered();
            break;
        
        default:
            microkit_dbg_puts("ERROR!\n");
            break;
    }

    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {};