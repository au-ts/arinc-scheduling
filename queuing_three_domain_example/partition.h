#pragma once

#include <stdint.h>

// All registers except thread ID
#define NUM_REG_SAVE (sizeof(seL4_UserContext)/sizeof(seL4_Word)) - 2 

typedef enum {
    INIT = 0, READY, RUNNING, OVERRUN, RECOVER, NOT_READY 
} PARTITION_STATE_TYPE;

/* Only scheduler has access to this */
typedef struct {
    uint32_t RELATIVE_START;
    uint32_t LENGTH;
    /* sPD ePD notification */
} PARTITION_ATTR_t;

typedef struct {
    PARTITION_STATE_TYPE pco_status;
    PARTITION_STATE_TYPE aco_status;
    microkit_cothread_ref_t pco_cothread_ref; 
    microkit_cothread_ref_t aco_cothread_ref; 
    void * pco_entry_point;
    void * aco_entry_point;
    uint64_t pco_sp_after_init;
    void * pco_recovery_fn;
    void * aco_recovery_fn;
    int preempted; // Specific to aCo, let's it know it was preempted
    int critical_section; // Specific to aCo for queueing port, will set this as it begins reading
} process_internal;

typedef struct {
    PARTITION_STATE_TYPE state;
    PARTITION_STATE_TYPE upd_state;
} partition_internal;

void set_partition_state(PARTITION_SHARED_t *partition, PARTITION_STATE_TYPE state);

// // Copy of 
// void set_tcb_pc(uint64_t tcb_id, seL4_Word pc) {
//     seL4_Error err;
//     seL4_UserContext ctxt = {0};
//     ctxt.pc = entry_point;
//     err = seL4_TCB_WriteRegisters(
//               BASE_TCB_CAP + pd,
//               seL4_True,
//               0, /* No flags */
//               1, /* writing 1 register */
//               &ctxt
//           );

//     if (err != seL4_NoError) {
//         microkit_dbg_puts("set_tcb_pc: error writing TCB registers\n");
//         microkit_internal_crash(err);
//     }
// }
