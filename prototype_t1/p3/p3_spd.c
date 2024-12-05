#include "p3.h"

#define SCHEDULER_CH_ID 4
#define PPD_CH_ID 7

/* CHANNEL PORT (Send of another partition) */
volatile SAMPLING_PORT_TYPE *P2_SEND_PORT;
volatile SAMPLING_PORT_TYPE *P1_BROADCAST_PORT;

/* RECEIVE PORT */
volatile SAMPLING_PORT_TYPE *P2_RECV;
volatile SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

/* PARTITION INFO */
volatile PARTITION_SHARED_t *P_STATE;
volatile PD_STATUS_t *PPD_STATUS;

void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P3 PPD READY, Initialising P3 SPD\n");
        P_STATE->state = READY;
        /* Set port status */
        reset_port(P2_RECV);
        reset_port(P1_BROADCAST_RECV);
    } 
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
    case SCHEDULER_CH_ID:

        /* Input processing of partition */
        /* Will overwrite existing message, otherwise existing message remains as per ARINC */
        check_set_message(P2_SEND_PORT, P2_RECV);
        check_set_message(P1_BROADCAST_PORT,P1_BROADCAST_RECV);

        /* Invoke pPD (donate scheduling context)*/
        microkit_ppcall(PPD_CH_ID, microkit_msginfo_new(0,0));

        /* If pPD returns, did not overrun */
        P_STATE->state = READY;

    }
};