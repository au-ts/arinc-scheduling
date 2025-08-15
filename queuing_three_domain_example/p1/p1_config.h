#pragma once
#include "../port.h"

#define SPD_CH_ID 5
#define UPD_TCB_ID 1

/* WCET of sPD implementing interpartition communication, and handling uPD */
#define PARTITION_SETUP_TIME 50

/* WCET of restoring aCo registers, and for the aCo to yield back to root thread */
#define RESTORE_ACO_TIME 50

SAMPLING_PORT_TYPE *P2_PORT;
SAMPLING_PORT_TYPE *BROADCAST_PORT;