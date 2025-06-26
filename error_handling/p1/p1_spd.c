#include "p1.h"

#define SCHEDULER_CH_ID 2
#define PPD_CH_ID 5
#define APD_CH_ID 8

/* PORTS */
SAMPLING_PORT_TYPE *SEND_P2_PORT;
SAMPLING_PORT_TYPE *SEND_ALL_PORT;
SAMPLING_PORT_TYPE *SEND_APD_PORT;

/* PARTITION INFO */

PARTITION_SHARED_t *P_STATE;
PD_STATUS_t *PPD_STATUS;
PD_STATUS_t *APD_STATUS;

void init(void) {
    if (PPD_STATUS->status == READY && APD_STATUS->status == READY) { 
        microkit_dbg_puts("P1 PPD READY, Initialising P1 SPD\n");
        // P_STATE->state = READY;
        /* Set init port status */
        reset_port(SEND_P2_PORT);
        reset_port(SEND_ALL_PORT);
        reset_port(SEND_APD_PORT);
    }
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
    case SCHEDULER_CH_ID:
        
        microkit_dbg_puts("P1 sPD\n");
        
        /* Set default, empty and invalid */
        /* We assume pPD follow protocol and sets message to be valid + full */
        reset_port(SEND_P2_PORT);
        reset_port(SEND_ALL_PORT);

        PPD_STATUS->status = RUNNING;
        /* Invoke pPD (donate scheduling context)*/
        // microkit_dbg_puts("P1 sPD call pPD\n");
        microkit_ppcall(PPD_CH_ID, microkit_msginfo_new(0,0));
        sddf_dprintf("After reply from PPD\n");

        // seL4_DebugDumpScheduler();

        if (PPD_STATUS->status == RECOVER) {
            sddf_dprintf("RECOVERY Completed for pPD\n"); 
            PPD_STATUS->status = READY;
            return;
        }

        PPD_STATUS->status = READY;

        sddf_dprintf("P1 status from sPD: %d\n", PPD_STATUS->status)

        APD_STATUS->status = RUNNING;
        // microkit_dbg_puts("P1 sPD call aPD\n");
        microkit_notify(APD_CH_ID);
 
    }
};


