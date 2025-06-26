#include "partition.h"

void set_partition_state(PARTITION_SHARED_t *partition, PARTITION_STATE_TYPE state) {
    partition->state = state;
}