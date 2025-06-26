#pragma once
#include <stdint.h>
typedef enum {
    VALID = 0, INVALID
} PORT_STATUS_TYPE;

typedef enum {
    FULL = 0, EMPTY
} PORT_EMPTY_TYPE;

typedef struct {
    PORT_STATUS_TYPE status;
    PORT_EMPTY_TYPE empty;
    int message;
} SAMPLING_PORT_TYPE;