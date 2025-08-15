#pragma once
#include "../port.h"

#define SPD_CH_ID 6
#define EPD_CH_ID 9

#define UPD_TCB_ID 1

/* WCET of sPD implementing interpartition communication, and handling uPD */
#define PARTITION_SETUP_TIME 50

/* WCET of restoring aCo registers, and for the aCo to yield back to root thread */
#define RESTORE_ACO_TIME 50

/* Send Port */
QUEUING_PORT_TYPE *P3_PORT;
/* Receive Port */
SAMPLING_PORT_TYPE *P1_RECV;
SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

