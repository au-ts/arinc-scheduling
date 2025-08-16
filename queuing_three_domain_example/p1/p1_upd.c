#include "p1.h"
#include "p1_config.h"
#include "upd_types.h"

/* Below here is generic for all partitions */

/* Buffer for message */
QUEUE_MSG buf = {0};

int first_run = 0;

process_internal *status;

partition_internal *partition_state;

void periodic_recovery(void);

void periodic(void);

void aperiodic(void);

void pco_recovery() {
    /* Do any handling actions as pCo will be destroyed when it yields. */
    periodic_recovery();
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    /* Notify ePD to restore the post-initialisation state of the pCo */
    microkit_notify(EPD_CH_ID);
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
    /* Init */
    periodic_init();
    /* Notify sPD to save post-initialisation context */
    microkit_notify(SPD_INIT_CH_ID);
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    
    /* Post-init */
    while (1) {
        periodic();
        microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    }
}

void aco_entry() {
    aperiodic_init();
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    aperiodic();
}


void init(void) {
    microkit_dbg_puts("Initialising P1 uPD\n");

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
    microkit_cothread_ref_t pco = microkit_cothread_spawn(status->pco_entry_point, PCO_ID);
    microkit_cothread_ref_t aco = microkit_cothread_spawn(status->aco_entry_point, ACO_ID);

    status->pco_cothread_ref = pco;
    status->aco_cothread_ref = aco;

    microkit_cothread_yieldto(pco);

    microkit_cothread_yieldto(aco);

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
            // If first time running, save pCo SP (which is SP after it finished init)
            if (first_run) {
                status->pco_sp_after_init = microkit_get_cothread_sp(status->pco_cothread_ref);
                first_run = 0;
            }

            /* If aCo in CS and reading, switch to aCo */
            if (aco_status->critical_section) {
                /* Set preempted flag so aCo yield after reading message */
                aco_status->preempted = 1; 
                microkit_cothread_yieldto(aco_status->cothread_ref);
            }

            /* Run periodic application code */
            pco_status->status = RUNNING;
            microkit_cothread_yieldto(pco_status->cothread_ref);

            /* Check if pCo was recovering */
            if (status->pco_status == RECOVER) { 
                status->pco_status = READY;
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