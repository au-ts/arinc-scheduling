#include "p1.h"
#include <stdint.h>

#define SPD_ID 1
#define PPD_ID 2
#define APD_ID 3

#define SCHEDULER_CH_ID 1
#define SCHEDULER_REBIND_CHANNEL 2

/* PARTITION INFO */

PARTITION_SHARED_t *P_STATE;
PD_STATUS_t *PPD_STATUS = {0};
PD_STATUS_t *APD_STATUS = {0};

int init_finished = 0;

void init(void) {
    sddf_dprintf("In ePD 1 init\n");
    /* User spare SC */
    microkit_dbg_puts("P1 ePD setting state to NOT_READY\n");
    P_STATE->state = NOT_READY;
};

void notified(microkit_channel ch) {
    sddf_dprintf("P1 EPD\n");
    // seL4_DebugDumpScheduler();

    switch (ch) {
        case SCHEDULER_CH_ID: {
            /* Init */
            /* Assign error handling functions after the PPD, SPD, APD have run */
            if (!init_finished) {
                microkit_dbg_puts("P1 ePD Initialising!\n");
                if (PPD_STATUS->handle_error_fn != 0) {
                    P_STATE->ppd_error_hdl = PPD_STATUS->handle_error_fn;
                }
                if (APD_STATUS->handle_error_fn != 0) {
                    P_STATE->apd_error_hdl = APD_STATUS->handle_error_fn;
                }
                P_STATE->state = READY;
                init_finished = 1;
                return;
            }
            /* Handling overruns */
            if (PPD_STATUS->status == RUNNING) {
                sddf_dprintf("PPD OVERRUN!!!\n");
                // microkit_dbg_puts("P1 ePD 2\n");
                P_STATE->recovering_pd = PPD_RECOVERING;
                PPD_STATUS->status = RECOVER;               

                /* Note: pPD is passive so does not have SC. sPD is also passive - do we need to unbind from sPD or it's notification */

                int res;

                /* Unbind sPD's notification SC from pPD */
                res = seL4_SchedContext_UnbindObject(BASE_SC_CAP + SPD_ID, BASE_TCB_CAP + PPD_ID);
                if (res != 0) {
                    microkit_dbg_puts("P1 EPD: Error unbinding SC from pPD!\n");
                }

                /* Bind sPD notification's SC back to itself */
                // res = seL4_SchedContext_Bind(BASE_SC_CAP + SPD_ID, BASE_NOTIFICATION_CAP + SPD_ID);
                // if (res != 0) {
                //     microkit_dbg_puts("P1 EPD: Error binding SC to notification\n");
                // }

                /* Bind spare SC to pPD */
                res = seL4_SchedContext_Bind(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID); 
                if (res != 0) {
                    microkit_dbg_puts("P1 EPD: Error binding spare SC to pPD!\n");
                }

                // Set PC of PPD to error handler function it registered
                microkit_pd_restart(PPD_ID, (seL4_Word) P_STATE->ppd_error_hdl);
                return;

            } else if (APD_STATUS->status == RUNNING) {
            /* Otherwise it's the aPD that's overrun (which is normal case)*/
            /* aPD has full SC so dont need to bind/unbind, just set PC to */
            /* recovery function and yield */
                P_STATE->recovering_pd = APD_RECOVERING;
                APD_STATUS->status = RECOVER;
                microkit_pd_restart(APD_ID, (uintptr_t) P_STATE->apd_error_hdl);
                return;
            } else {
            /* No aPD - do nothing */
                // microkit_dbg_puts("P1 ePD 4\n");
            } 
            break;
        }
        /* We are unbinding spare from pPD */
        case SCHEDULER_REBIND_CHANNEL: {
            int res = seL4_SchedContext_UnbindObject(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID);
            if (res != 0) {
                microkit_dbg_puts("P1 EPD: Error unbinding spare SC from pPD!\n");
            }
            
            P_STATE->recovering_pd = 0;
            // seL4_DebugSnapshot();
            break;
        }

    }
};

seL4_Bool fault(microkit_child child, microkit_msginfo msginfo, microkit_msginfo *reply_msginfo) {
    return seL4_False;
}


