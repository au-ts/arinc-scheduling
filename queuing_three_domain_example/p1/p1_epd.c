#include "p1.h"
#include <stdint.h>

#define SPD_ID 1
#define UPD_ID 2

#define SCHEDULER_CH_ID 1
#define SCHEDULER_REBIND_CHANNEL 2
#define UPD_CH_ID 8

/* PARTITION INFO */

partition_internal *P_STATE;
process_internal *PPD_STATUS = {0};
process_internal *APD_STATUS = {0};

seL4_UserContext aco_regs;

int init_finished = 0;

void init(void) {
    sddf_dprintf("In ePD 1 init\n");
    /* User spare SC */
    microkit_dbg_puts("P1 ePD setting state to NOT_READY\n");
    P_STATE->state = NOT_READY;
};

void notified(microkit_channel ch) {
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
                
                /* Set pCo to recovery function. Will be destroyed after */

                P_STATE->pco.state = RECOVER;
                PPD_STATUS->status = RECOVER;               

                /* Note: pPD is passive so does not have SC. sPD is also passive - do we need to unbind from sPD or it's notification */

                /* Unbind sPD's notification SC from pPD */
                int res = seL4_SchedContext_UnbindObject(BASE_SC_CAP + SPD_ID, BASE_TCB_CAP + PPD_ID);
                if (res != 0) {
                    microkit_dbg_puts("P1 EPD: Error unbinding SC from pPD!\n");
                }

                /* Bind sPD notification's SC back to itself */
                res = seL4_SchedContext_Bind(BASE_SC_CAP + SPD_ID, BASE_NOTIFICATION_CAP + SPD_ID);
                if (res != 0) {
                    microkit_dbg_puts("P1 EPD: Error binding PPD\n");
                }

                /* Bind spare SC to pPD */
                res = seL4_SchedContext_Bind(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID); 
                if (res != 0) {
                    microkit_dbg_puts("P1 EPD: Error binding spare SC to pPD!\n");
                }

                // WARNING check recovery_fn
                // Set PC of PPD to error handler function it registered
                microkit_pd_restart(UPD_ID, (seL4_Word) P_STATE->pco.recovery_fn);

                return;

            } else if (APD_STATUS->status == RUNNING) {
            /* Otherwise it's the aCo that's overrun (which is normal case)*/
            /* Just needs to aCo to return back to root thread */
                P_STATE->aco.status = RECOVER;
                APD_STATUS->status = RECOVER;
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, &aco_regs); // Save current PC of Client
                // WARNING check recovery_fn- recovery function here saves registers
                microkit_pd_restart(UPD_ID, (seL4_Word) P_STATE->aco.recovery_fn);
                microkit_notify(UPD_CH_ID);
                return;
            } else {
            /* No aPD - do nothing */
            } 
            break;
        }
        /* We are unbinding spare from pPD */
        case SCHEDULER_REBIND_CHANNEL: {
            int res = seL4_SchedContext_UnbindObject(SPARE_SC_CAP, BASE_TCB_CAP + PPD_ID);
            if (res != 0) {
                microkit_dbg_puts("P1 EPD: Error unbinding spare SC from pPD!\n");
            }
            // microkit_dbg_puts("P1 ePD 5\n");
            P_STATE->recovering_pd = 0;
            break;
        }

        case UPD_CH_ID: {
            // Restore registers of aCo
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, &aco_regs); 
            APD_STATUS->status = READY;
        }

    }
};

seL4_Bool fault(microkit_child child, microkit_msginfo msginfo, microkit_msginfo *reply_msginfo) {
    return seL4_False;
}

