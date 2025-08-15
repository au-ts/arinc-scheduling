#include "p2.h"
#include "p2_config.h"
#include "upd_types.h"

/* Below here is generic for all partitions */

/* Buffer for message */
QUEUE_MSG buf = {0};

process_internal *status;

partition_internal *partition_state;

void periodic_recovery(void);

void periodic(void);

void aperiodic(void);

void pco_recovery() {
    /* Do any handling actions as pCo will be destroyed when it yields. */
    periodic_recovery();
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
}

void aco_handle() {
    /* Yield to root co-thread */
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);

    /* After regaining control, signal ePD to restore registers*/
    /* i.e. back to where it was before it was preempted */
    microkit_notify(SPD_CH_ID);

    /* Since the ePD is ready/schedulable and strictly higher prio */
    /* The context switch is immediate, so any lines after notify do not execute*/
    /* Importantly, this function will not return. */
}

void pco_entry() {
    periodic();
}

void aco_entry() {
    aperiodic();
}


void init(void) {
    microkit_dbg_puts("Initialising P1 uPD\n");

    // old
    user_init();
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

    pco_status->entry_point = &pco_entry;
    aco_status->entry_point = &aco_entry;

    pco_status->recovery_fn = &pco_recovery;
    aco_status->recovery_fn = &aco_recovery;

    pco_status->status = READY;
    aco_status->status = READY;

    // WARNING
    microkit_cothread_ref_t aco = microkit_cothread_spawn(aco_status->entry_point, ACO_ID);

    aco_status->cothread_ref = aco;

    // Any init

}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    switch (channel) { 
        default:
        microkit_dbg_puts("ERROR!\n");
            break;
    }
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {
    switch (ch) {
        case SPD_CH_ID: { 
            /* Basically we also need to restore the aCo registers before we switch to it. */
            /* This could either be when it finishes reading messages */
            /* Or when it is scheduled after pCo*/

            /* If aCo in CS and reading, switch to aCo */
            if (aco_status->critical_section) {
                /* Set preempted flag so aCo yield after reading message */
                aco_status->preempted = 1; 
                microkit_cothread_yieldto(aco_status->cothread_ref);
            }

            /* Spawn pCo */
            microkit_cothread_ref_t pco = microkit_cothread_spawn(pco_status->entry_point, PCO_ID);
            pco_status->cothread_ref = pco;
            pco_status->status = ready;

            /* Run periodic application code */
            pco_status->status = RUNNING;
            microkit_cothread_yieldto(pco_status->cothread_ref);
            /* Destroy pCo upon finish */
            microkit_cothread_destroy(pco_status->cothread_ref);
            /* Check if pCo was recoverying */
            if (pco_status->status == RECOVER) { 
                /* Unblock the sPD */
                break;
            }

            /* Back from periodic, run aperiodic */
            aco_status->status = RUNNING;
            microkit_cothread_yieldto(aco_status->cothread_ref);

            /* After aCo handling, set status to ready, and block on notification */
            aco_status->status = READY;

            break;
        }
    }
};