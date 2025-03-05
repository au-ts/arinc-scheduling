#include "partition.h"

void set_partition_state(volatile PARTITION_SHARED_t *partition, PARTITION_STATE_TYPE state) {
    partition->state = state;
}