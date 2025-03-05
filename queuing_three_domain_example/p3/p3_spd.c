#include "p3.h"

#define SCHEDULER_CH_ID 4
#define PPD_CH_ID 7

/* CHANNEL PORT (Send of another partition) */
QUEUING_PORT_TYPE *P2_SEND_PORT;
SAMPLING_PORT_TYPE *P1_BROADCAST_PORT;

/* RECEIVE PORT */
QUEUING_PORT_TYPE *P2_RECV;
SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

/* PARTITION INFO */
PARTITION_SHARED_t *P_STATE;
PD_STATUS_t *PPD_STATUS;

void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P3 PPD READY, Initialising P3 SPD\n");
        P_STATE->state = READY;
        /* Set port status */
        init_queuing_port(P2_RECV);
        reset_sampling_port(P1_BROADCAST_RECV);
    } 
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
    case SCHEDULER_CH_ID:

        /* Input processing from P2 send buffer to P3 receive buffer */
        transfer_queuing_buffers(P2_SEND_PORT, P2_RECV);
        /* Input processing of sampling port from P1 */
        check_set_message(P1_BROADCAST_PORT,P1_BROADCAST_RECV);

        /* Invoke pPD (donate scheduling context)*/
        microkit_ppcall(PPD_CH_ID, microkit_msginfo_new(0,0));

        /* If pPD returns, did not overrun */
        P_STATE->state = READY;

    }
};