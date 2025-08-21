#pragma once
#include "../port.h"

/* WCET of sPD implementing interpartition communication, and handling uPD */
#define PARTITION_SETUP_TIME 50 * NS_IN_MS

/* WCET of restoring aCo registers, and for the aCo to yield back to root thread */
#define RESTORE_ACO_TIME 50 * NS_IN_MS

SAMPLING_PORT_TYPE *P2_PORT;
SAMPLING_PORT_TYPE *BROADCAST_PORT;