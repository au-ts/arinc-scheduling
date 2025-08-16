#include "p1.h"
#include "p1_config.h"

#define TIMER_CH_ID 1
#define SCHEDULER_CH_ID 2
#define EPD_CH_ID 3
#define UPD_CH_ID 4
#define UPD_PCO_INIT_CH 5

/* PORTS */
SAMPLING_PORT_TYPE *SEND_P2_PORT;
SAMPLING_PORT_TYPE *SEND_ALL_PORT;

/* PARTITION INFO */

partition_internal *partition_state;
/* TODO map memory for aco_status/pco_status */
process_internal *upd_status;

/* TODO map memory for regs */
seL4_UserContext *aco_regs;

seL4_UserContext *pco_ctxt;

int aco_flag = 0;


void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P1 PPD READY, Initialising P1 SPD\n");
        partition_state->state = READY;

        /* Initialise ports */
    }
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
                /* Assume suspend has been done by scheduler */
                
                /* Save registers of the uPD */
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, aco_regs); // Save current PC of Client

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
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, aco_regs); 
            /* Reset saved aCo context */
            aco_regs = {0};
            aco_status->status = READY;

            break;
        }

        case UPD_PCO_INIT_CH: {
            /* pCo has finished its init. Save its context */
            seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, pco_ctxt);
        } 
    }
};


