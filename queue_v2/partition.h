#pragma once

#include <stdint.h>

#define RECOVERY_MASK 3
#define PPD_RECOVERING 1
#define APD_RECOVERING 2

typedef enum {
    INIT = 0, READY, RUNNING, OVERRUN, RECOVER, NOT_READY 
} PARTITION_STATE_TYPE;

typedef struct {
    PARTITION_STATE_TYPE state;
    int recovering_pd;
    void * ppd_error_hdl;
    void * apd_error_hdl;
} PARTITION_SHARED_t;

/* Only scheduler has access to this */
typedef struct {
    uint32_t RELATIVE_START;
    uint32_t LENGTH;
    /* sPD ePD notification */
} PARTITION_ATTR_t;

typedef struct {
    PARTITION_STATE_TYPE status;
    void * handle_error_fn;
} PD_STATUS_t;

void set_partition_state(PARTITION_SHARED_t *partition, PARTITION_STATE_TYPE state);