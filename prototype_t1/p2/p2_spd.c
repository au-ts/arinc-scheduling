#include "p2.h"

#define SCHEDULER_CH_ID 3
#define PPD_CH_ID 6

/* SEND PORT */
volatile SAMPLING_PORT_TYPE *SEND_P3_PORT;

/* CHANNEL PORT (Send port of another partition) */
volatile SAMPLING_PORT_TYPE *P1_SEND_PORT;
volatile SAMPLING_PORT_TYPE *P1_BROADCAST_PORT;

/* RECEIVE PORT (The ports accessible to all applications in this application) */
volatile SAMPLING_PORT_TYPE *P1_RECV;
volatile SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

volatile PARTITION_SHARED_t *P_STATE;
volatile PD_STATUS_t *PPD_STATUS;

void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P2 PPD READY, Initialising P1 SPD\n");
        P_STATE->state = READY;
        reset_port(SEND_P3_PORT);
        reset_port(P1_RECV);
        reset_port(P1_BROADCAST_RECV);
    }
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
    case SCHEDULER_CH_ID:

        /* Input processing of partition */
        /* Will overwrite existing message, otherwise existing message remains as per ARINC */
        check_set_message(P1_SEND_PORT, P1_RECV);
        check_set_message(P1_BROADCAST_PORT, P1_BROADCAST_RECV);

        /* Assume pPD will set to valid + full if writing to port */
        reset_port(SEND_P3_PORT);

        /* Invoke pPD (donate scheduling context)*/
        microkit_ppcall(PPD_CH_ID, microkit_msginfo_new(0,0));

        /* If pPD returns, did not overrun */
        P_STATE->state = READY;

    }
};