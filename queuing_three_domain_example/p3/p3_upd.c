#include "p3.h"
#include "upd_types.h"
#include "upd_shared.h"

/* Below here is generic for all partitions */

/* Buffer for message */
QUEUE_MSG buf = {0};

process_internal *status;

char * pco_stack;
char * aco_stack;

co_control_t co_control_mem;

void periodic_recovery(void);

void periodic(void);
void aperiodic(void);

void periodic_init(void);
void aperiodic_init(void);

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

void pco_init_entry() {
    periodic_init();
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
}

void aco_init_entry() {
    aperiodic_init();
    microkit_cothread_yieldto(ROOT_COTHREAD_REF);
}

void pco_entry() {
    /* Post-init */
    while (1) {
        periodic();
        microkit_cothread_yieldto(ROOT_COTHREAD_REF);
    }
}

void aco_entry() {
    // microkit_cothread_yieldto(ROOT_COTHREAD_REF);

    aperiodic();
}


void init(void) {
    microkit_dbg_puts("Initialising P3 uPD\n");

    stack_ptrs_arg_array_t stack_ptrs = {
        (uintptr_t) pco_stack,
        (uintptr_t) aco_stack,
    };
    microkit_cothread_init(
        &co_control_mem, 
        COSTACK_SIZE,
        stack_ptrs
    );

    status->pco_entry_point = &pco_entry;
    status->aco_entry_point = &aco_entry;

    status->pco_recovery_fn = &pco_recovery;
    status->aco_recovery_fn = &aco_handle;

    status->pco_status = READY;
    status->aco_status = READY;

    // WARNING
    microkit_cothread_ref_t pco = microkit_cothread_spawn(&pco_init_entry, (void *) PCO_ID);

    microkit_cothread_ref_t aco = microkit_cothread_spawn(&aco_init_entry, (void *) ACO_ID);

    if (microkit_cothread_spawn(&pco_entry, (void *) 3) != LIBMICROKITCO_NULL_HANDLE) {
        printf("ERR: was able to spawn more cothreads than allowed\n");
        return;
    }

    status->pco_cothread_ref = pco;
    status->aco_cothread_ref = aco;

    microkit_cothread_yieldto(pco); 
    microkit_cothread_destroy(pco);
    microkit_cothread_ref_t post_pco = microkit_cothread_spawn(&pco_entry, (void *) PCO_ID);

    if (!(pco == post_pco)) {
        printf("ERROR: pCo cothread_ref mismatch!\n");
    }

    status->pco_status = READY;

    microkit_cothread_yieldto(aco);
    microkit_cothread_destroy(aco);
    microkit_cothread_ref_t post_aco = microkit_cothread_spawn(&aco_entry, (void *) ACO_ID);

    if (!(aco == post_aco)) {
        printf("ERROR: aCo cothread_ref mismatch!\n");
    }

    status->aco_status = READY;

    microkit_notify(SPD_FINISH_INIT_CH_ID);
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
            // // If first time running, save pCo SP (which is SP after it finished init)
            // if (first_run) {
            //     status->pco_sp_after_init = microkit_get_cothread_sp(status->pco_cothread_ref);
            //     first_run = 0;
            // }

            microkit_dbg_puts("uPD 3 notified from sPD\n");

            /* If aCo in CS and reading, switch to aCo */
            if (status->critical_section) {
                /* Set preempted flag so aCo yield after reading message */
                status->preempted = 1; 
                microkit_cothread_yieldto(status->aco_cothread_ref);
            }

            /* Run periodic application code */
            status->pco_status = RUNNING;
            microkit_cothread_yieldto(status->pco_cothread_ref);
            /* Check if pCo was recovering */
            if (status->pco_status == RECOVER) { 
                status->pco_status = READY;
                /* Unblock the sPD */
                break;
            }
            status->pco_status = READY;

            /* Back from periodic, run aperiodic */
            status->aco_status = RUNNING;
            microkit_cothread_yieldto(status->aco_cothread_ref);

            /* After aCo handling, set status to ready, and block on notification */
            status->aco_status = READY;

            break;
        }
    }
};