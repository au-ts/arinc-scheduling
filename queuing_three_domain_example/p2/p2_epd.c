#include "p2.h"
#include "p2_config.h"

#define SPD_ID 1
#define UPD_ID 2

#define SCHEDULER_CH_ID 1
#define SCHEDULER_REBIND_CHANNEL 2
#define UPD_CH_ID 8

/* PARTITION INFO */

partition_internal *partition_state;
process_internal *upd_status;

int init_finished = 0;

void init(void) {
    sddf_dprintf("In ePD 1 init\n");
    /* User spare SC */
    microkit_dbg_puts("P1 ePD setting state to NOT_READY\n");
    P_STATE->state = NOT_READY;
};

void notified(microkit_channel ch) {
    switch (ch) {

        /* Init */
        case SCHEDULER_CH_ID: {
            /* Assign error handling functions after the PPD, SPD, APD have run */
            if (!init_finished) {
                microkit_dbg_puts("P1 ePD Initialising!\n");
                if (pco_status->handle_error_fn != 0) {
                    P_STATE->ppd_error_hdl = PPD_STATUS->handle_error_fn;
                }
                if (aco_status->handle_error_fn != 0) {
                    P_STATE->apd_error_hdl = APD_STATUS->handle_error_fn;
                }
                P_STATE->state = READY;
                init_finished = 1;
                return;
            }
            break;
        }

        /* Handling pCo overruns */
        case SPD_CH_ID: {
            if (pco_status->status == RUNNING) { 
                printf("Starting uPD recovery!\n");

                partition_state->state = RECOVER;
                partition_state->upd_state = RECOVER;
                pco_status->status = RECOVER;               

                // WARNING check recovery_fn
                // Set PC of PPD to error handler function it registered
                microkit_pd_restart(UPD_ID, (seL4_Word) PPD_STATUS->recovery_fn);

                /* Resume uPD TCB */
                seL4_TCB_Resume();

                return;

            } else if (pco_status == RECOVER) {
                /* If already recovering, just let uPD continue */
                seL4_TCB_Resume();
                printf("Continuing uPD recovery!\n");
                break;

            } else {
                printf("ePD notified by sPD but no overrun or recovery in progress?\n");
                break;
            }
            


            break;
        }
  
    }
};

seL4_Bool fault(microkit_child child, microkit_msginfo msginfo, microkit_msginfo *reply_msginfo) {
    return seL4_False;
}

