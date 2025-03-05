#include "p3.h"

#define SPD_CH_ID 7
volatile PD_STATUS_t *STATUS;

/* RECEIVE PORT */
volatile SAMPLING_PORT_TYPE *P2_RECV;
volatile SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

void p3_initialize(void);
void p3_timeTriggered(void);

int read_p2(void) {
    return read_from_port(P2_RECV);
}

int read_broadcast(void) {
    return read_from_port(P1_BROADCAST_RECV);
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