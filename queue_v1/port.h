#pragma once
#include <stdint.h>
#define QUEUING_BUFFER_LEN 3
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

typedef struct {
    int data;
    PORT_STATUS_TYPE status; /* Used to indicate message has been moved atomically */
} QUEUE_MSG;

typedef struct {
    int max_len;
    int head;
    int size;
    QUEUE_MSG buffer[QUEUING_BUFFER_LEN];  
} QUEUING_PORT_TYPE;