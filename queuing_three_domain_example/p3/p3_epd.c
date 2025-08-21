#include "p3.h"
#include "epd_shared.h"

#define SCHEDULER_CH_ID 1
#define SCHEDULER_REBIND_CHANNEL 2

/* PARTITION INFO */

partition_internal *partition_state;
process_internal *upd_status;

/*  for pCo */
seL4_UserContext *pco_ctxt;

int init_finished = 0;

void init(void) {
    printf("In ePD 3 init\n");
};

void notified(microkit_channel ch) {
    switch (ch) {
        /* Handling pCo overruns */
        case SPD_CH_ID: {
            if (upd_status->pco_status == RUNNING) { 
                printf("Starting uPD recovery!\n");

                partition_state->state = RECOVER;
                partition_state->upd_state = RECOVER;
                // pco_status->status = RECOVER;               

                // WARNING check recovery_fn
                // Set PC of PPD to error handler function it registered
                microkit_pd_restart(UPD_TCB_ID, (seL4_Word) upd_status->pco_recovery_fn);


                /* Resume uPD TCB */
                seL4_TCB_Resume(BASE_TCB_CAP + UPD_TCB_ID);

                break;

            } else if (upd_status->pco_status == RECOVER) {
                /* If already recovering, just let uPD continue */
                seL4_TCB_Resume(BASE_TCB_CAP + UPD_TCB_ID);
                printf("Continuing uPD recovery!\n");
                break;

            } else {
                printf("ePD notified by sPD but no overrun or recovery in progress?\n");
                break;
            }
            


            break;
        }

        case UPD_CH_ID: {
            /* Restore aCo registers */
            // seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, pco_ctxt); 
            microkit_pd_restart(UPD_TCB_ID, (seL4_Word) upd_status->pco_entry_point);
            break;
        }
  
    }
};


