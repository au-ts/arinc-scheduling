#include "p1.h"
#include "upd_types.h"

#define SPD_CH_ID 5
#define EPD_CH_ID 8

SAMPLING_PORT_TYPE *P2_PORT;
SAMPLING_PORT_TYPE *BROADCAST_PORT;

process_internal *pco_status;
process_internal *aco_status;

partition_internal *partition_state;

void p1_initialize(void);
void p1_timeTriggered(void);

void send_p2(int message) {
    write_sampling_message(P2_PORT, message);
}

void broadcast(int message) {
    write_sampling_message(BROADCAST_PORT, message);
}

void p1_pco_recovery() {
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
}

void p1_aco_recovery() {
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
}

void p1_pco_entry() {

}

void p1_aco_entry() {

}


void init(void) {
    microkit_dbg_puts("Initialising P1 uPD\n");

    // old
    p1_initialize();
    //

    stack_ptrs_arg_array_t stack_ptrs = {
        (uintptr_t) &stack1,
        (uintptr_t) &stack2,
    };
    microkit_cothread_init(
        &co_control_mem, 
        COSTACK_SIZE,
        stack_ptrs
    );

    pco_status->entry_point = &p1_pco_entry;
    aco_status->entry_point = &p1_aco_entry;

    pco_status->recovery_fn = &p1_pco_recovery;
    aco_status->recovery_fn = &p1_aco_recovery;

    pco_status->status = READY;
    aco_status->status = READY;

    // WARNING
    microkit_cothread_ref_t aco = microkit_cothread_spawn(aco_status->entry_point, ACO_ID);

    partition_state->pco = *pco_status;
    partition_state->aco = *aco_status;
    partition_state->aco->cothread_ref = aco;

    // Any init

}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    switch (channel) {
        case SPD_CH_ID:    
            /* Basically we also need to restore the aCo registers before we switch to it. */
            /* This could either be when it finishes reading messages */
            /* Or when it is scheduled after pCo*/

            /* Insert aCo status check */
            /* If in CS and reading, switch to aCo */

            /* Spawn pCo */
            microkit_cothread_ref_t pco = microkit_cothread_spawn(pco_status->entry_point, PCO_ID);
            partition_state->pco_ref = pco;

            /* Run periodic application code */
            partition_state->pco.status = RUNNING;
            microkit_cothread_yieldto(partition_state->pco_ref);
            /* Destroy pCo upon finish */
            microkit_cothread_destroy(partition_state->pco.ref);
            /* Check if pCo was recoverying */
            if (partition_state->pco.status == RECOVER) {
                break;
            }
            partition_state->pco.status = READY;

            /* Back from periodic, run aperiodic */
            /* If aCo is not_ready, restore registers first */
            /* Notify ePD who will restore registers */
            if (partition_state->aco.status == NOT_READY) {
                microkit_notify(EPD_CH_ID);      
            }
            /* Make sure aCo actually ready */
            if (partition_state->aco.status != READY) {
                printf("ERROR!: aCo register restore not done properly!\n");
                break;
            }
            partition_state->aco.status = RUNNING;
            microkit_cothread_yieldto(partition_state->aco_ref);
            break;
        
        default:
        microkit_dbg_puts("ERROR!\n");
            break;
    }
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {
    switch (ch) {
        // Mark as not_ready from recovering after aPD co-switches to root thread
        // aCo needs to have registers restored before it can run again.
        // uPD can receive signals again 
        case EPD_CH_ID: {
            partition_state->state = READY;
            partition_state->upd_state = READY;
            aco_status->status = NOT_READY;
        }
    }
};