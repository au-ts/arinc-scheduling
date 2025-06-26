#include "p1.h"

#define SPD_CH_ID 5

SAMPLING_PORT_TYPE *P2_PORT;
SAMPLING_PORT_TYPE *BROADCAST_PORT;

PD_STATUS_t *STATUS;

void p1_initialize(void);
void p1_timeTriggered(void);

void send_p2(int message) {
    write_sampling_message(P2_PORT, message);
}

void broadcast(int message) {
    write_sampling_message(BROADCAST_PORT, message);
}


void init(void) {
    microkit_dbg_puts("Initialising P1 PPD\n");
    p1_initialize();
    STATUS->status = READY;
}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    switch (channel) {
        case SPD_CH_ID:    
            /* Run periodic application code */
            p1_timeTriggered();

            break;
        
        default:
        microkit_dbg_puts("ERROR!\n");
            break;
    }
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {};