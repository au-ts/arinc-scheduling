#include "p1.h"


#define SPD_ID 1
#define PPD_ID 2
#define APD_ID 3

#define SCHEDULER_REBIND_CHANNEL 

/* PARTITION INFO */

PARTITION_SHARED_t *P_STATE;
PD_STATUS_t *PPD_STATUS = {0};
PD_STATUS_t *APD_STATUS = {0};

int init_finished = 0;

void init(void) {
    /* User spare SC */
    P_STATE->state = NOT_READY;
};

void notified(microkit_channel ch) {
    switch (ch) {
        case SCHEDULER_CH_ID:
            /* Init */
            /* Assign error handling functions after the PPD, SPD, APD have run */
            if (!init_finished) {
                if (PPD_STATUS->handle_error_fn != 0) {
                    P_STATE->ppd_error_hdl = PPD_STATUS->handle_error_fn;
                }
                P_STATE->state = READY;
                init_finished = 1;
                return;
            }

            /* Handling overruns */
            if (PPD_STATUS->status == RUNNING) {
                PPD_STATUS->status = RECOVER;               

                /* Note: pPD is passive so does not have SC. sPD is also passive - do we need to unbind from sPD or it's notification */

                /* Unbind sPD's notification SC from pPD */
                int res = seL4_SchedContext_UnbindObject(BASE_SC_CAP + SPD_ID, BASE_TCB_CAP + PPD_ID);
                if (res != 0) {
                    microkit_dbg_puts("Error unbinding SC from pPD!");
                }

                /* Bind sPD notification's SC back to itself */
                res = seL4_SchedContext_Bind(BASE_SC_CAP + SPD_ID, BASE_NOTIFICATION_CAP + SPD_ID);
                if (res != 0) {
                    microkit_dbg_puts("Error binding PPDkkk");
                }

                /* Bind spare SC to pPD */
                res = seL4_SchedContext_Bind(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID); 
                if (res != 0) {
                    microkit_dbg_puts("Error binding spare SC to pPD!");
                }
                return;

                // Set PC of PPD to error handler function it registered
                microkit_pd_restart(PPD_ID, P_STATE->ppd_error_hdl);

            } else {
            /* Otherwise it's the aPD that's overrun (which is normal case)*/
            /* aPD has full SC so dont need to bind/unbind, just set PC to */
            /* recovery function and yield */
                APD_STATUS->status = RECOVER;
                microkit_pd_restart(APD_ID, P_STATE->apd_error_hdl);
                return;
             }
             
        /* We are unbinding spare from pPD */
        case SCHEDULER_REBIND_CHANNEL:
            int res = seL4_SchedContext_UnbindObject(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID);
            break;

    }
};


