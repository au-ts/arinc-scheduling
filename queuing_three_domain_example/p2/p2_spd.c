#include "p2.h"

#define TIMER_CH_ID 1
#define SCHEDULER_CH_ID 2
#define EPD_CH_ID 3
#define UPD_CH_ID 4


/* SEND PORT */
QUEUING_PORT_TYPE *SEND_P3_PORT;

/* CHANNEL PORT (Send port of another partition) */
SAMPLING_PORT_TYPE *P1_SEND_PORT;
SAMPLING_PORT_TYPE *P1_BROADCAST_PORT;

/* RECEIVE PORT (The ports accessible to all applications in this application) */
SAMPLING_PORT_TYPE *P1_RECV;
SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

/* PARTITION INFO */

partition_internal *partition_state;
/* TODO map memory for aco_status/pco_status */
process_internal *upd_status;

/* TODO map memory for regs */
seL4_UserContext *aco_regs;

int aco_flag = 0;

void init(void) {
    if (PPD_STATUS->status == READY) { 
        microkit_dbg_puts("P2 PPD READY, Initialising P1 SPD\n");
        P_STATE->state = READY;
        init_queuing_port(SEND_P3_PORT);
        reset_sampling_port(P1_RECV);
        reset_sampling_port(P1_BROADCAST_RECV);
    }
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
        case SCHEDULER_CH_ID: {

            /* Timeout for partition setup */
            sddf_timer_set_timeout(TIMER_CH_ID, PARTITION_SETUP_TIME);

            /* Interpartition Communication Semantics */

            /* Will overwrite existing message, otherwise existing message remains as per ARINC */
            check_set_message(P1_SEND_PORT, P1_RECV);
            check_set_message(P1_BROADCAST_PORT, P1_BROADCAST_RECV);

            /* If the aCo running (normal operation after first-run), cleanup uPD */
            if (aco_status->status == RUNNING) {
                /* Assume suspend has been done by scheduler */
                
                /* Save registers of the uPD */
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, &aco_regs); // Save current PC of Client

                /* Resume uPD from aCo handling fn */
                // WARNING check recovery_fn- recovery function here saves registers
                microkit_pd_restart(UPD_ID, (seL4_Word) APD_STATUS->recovery_fn);

                /* Resume the suspended uPD */
                seL4_TCB_Resume();
            }
            
            break;
        }

        case TIMER_CH_ID: {
        /* Partition setup complete */ 
            partition_state->state = RUNNING;
            if (aco_status->status != READY) {
                printf("ERROR!: aCo did not finish handling!\n");
                break;
            } 
            /* Notify uPD's root co-thread */
            microkit_notify(UPD_CH_ID);

            break;
        }

        case UPD_CH_ID: {

            /* If not enough time remaining to restore registers, and yield, set flag and skip */
            if (!(sddf_timer_time_now(TIMER_CH_ID))) {
                aco_flag = 1;
                break;
            }

            /* Restore aCo registers */
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_ID, seL4_False, 0, NUM_REG_SAVE, &aco_regs); 
            /* Reset saved aCo context */
            aco_regs = {0};
            aco_status->status = READY;

            break;
        }
    }
};