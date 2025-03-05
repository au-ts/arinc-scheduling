#include "p1.h"

#define SCHEDULER_CH_ID 2
#define PPD_CH_ID 5

/* PORTS */
SAMPLING_PORT_TYPE *SEND_P2_PORT;
SAMPLING_PORT_TYPE *SEND_ALL_PORT;

/* PARTITION INFO */

PARTITION_SHARED_t *P_STATE;
PD_STATUS_t *PPD_STATUS;

void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P1 PPD READY, Initialising P1 SPD\n");
        P_STATE->state = READY;
        /* Set init port status */
        reset_sampling_port(SEND_P2_PORT);
        reset_sampling_port(SEND_ALL_PORT);
    }
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
    case SCHEDULER_CH_ID:
        
        /* Set default, empty and invalid */
        /* We assume pPD follow protocol and sets message to be valid + full */
        reset_sampling_port(SEND_P2_PORT);
        reset_sampling_port(SEND_ALL_PORT);

        /* Invoke pPD (donate scheduling context)*/
        microkit_ppcall(PPD_CH_ID, microkit_msginfo_new(0,0));
 
        /* If pPD returns, did not overrun */
        P_STATE->state = READY;
    }
};


