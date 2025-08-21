#include "p3.h"
#include "p3_config.h"
#include "spd_shared.h"
#include <sddf/timer/client.h>

/* PORTS */
SAMPLING_PORT_TYPE *SEND_P2_PORT;
SAMPLING_PORT_TYPE *SEND_ALL_PORT;

/* PARTITION INFO */

partition_internal *partition_state;
/* TODO map memory for aco_status/pco_status */
process_internal *upd_status;

/* TODO map memory for regs */
seL4_UserContext *aco_ctxt;

seL4_UserContext *pco_ctxt;

int aco_flag = 0;


void init(void) { 
    /* sPD setup */

    printf("P3 SPD INIT\n");
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
        case SCHEDULER_CH_ID: {

            /* Check if partition overrun, and signal ePD instead */
            if (upd_status->pco_status == RUNNING || upd_status->pco_status == RECOVER) {
                microkit_notify(EPD_CH_ID);
                break;
            }

            /* Timeout for partition setup */
            sddf_timer_set_timeout(TIMER_CH_ID, PARTITION_SETUP_TIME);

            /* Interpartition Communication Semantics */
    
            /* If the aCo running (normal operation after first-run), cleanup uPD */
            if (upd_status->aco_status == RUNNING) {
                
                /* Save registers of the uPD */
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); // Save current PC of Client

                /* Resume uPD from aCo handling fn */
                microkit_pd_restart(UPD_TCB_ID, (seL4_Word) upd_status->aco_recovery_fn);

                /* Resume the suspended uPD */
                seL4_TCB_Resume(BASE_TCB_CAP + UPD_TCB_ID);
            }
            
            break;
        }

        case TIMER_CH_ID: {
        /* Partition setup complete */ 
            partition_state->state = RUNNING;
            if (upd_status->aco_status != READY) {
                printf("ERROR!: aCo did not finish handling!\n");
                break;
            } 
            /* Notify uPD's root co-thread */
            microkit_notify(UPD_CH_ID);

            break;
        }

        case UPD_CH_ID: {
            /* If not enough time remaining to restore registers, and yield, set flag and skip */
            // if (!(sddf_timer_time_now(TIMER_CH_ID))) {
            //     aco_flag = 1;
            //     break;
            // }

            /* Restore aCo registers */
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); 
            /* Reset saved aCo context */
            memset(aco_ctxt, 0, sizeof(*aco_ctxt));
            upd_status->aco_status = READY;

            break;
        }

        case UPD_PCO_INIT_CH: {
            /* pCo has finished its init. Save its context */
            microkit_dbg_puts("sPD 3: Saving pCo registers\n");
            seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, pco_ctxt);
            break;
        } 

        case UPD_FINISH_INIT_CH: {
            partition_state->state = READY;
            partition_state->upd_state = READY;
            microkit_dbg_puts("FINISH INIT UPD 3\n");
            break;
        }

        default: {
            microkit_dbg_puts("ERROR sPD 3: invalid channel\n");
        }
    }
};