#pragma once

#include <stdint.h>

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
    PARTITION_STATE_TYPE status;
    microkit_cothread_ref_t cothread_ref; 
    void * entry_point;
    void * recovery_fn;
    int preempted; // Specific to aCo, let's it know it was preempted
    int critical_section; // Specific to aCo for queueing port, will set this as it begins reading
} process_internal;

typedef struct {
    PARTITION_STATE_TYPE state;
    PARTITION_STATE_TYPE upd_state;
    process_internal pco;
    process_internal aco;
} partition_internal;

void set_partition_state(PARTITION_SHARED_t *partition, PARTITION_STATE_TYPE state);