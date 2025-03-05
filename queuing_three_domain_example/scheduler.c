#include "partition.h"
#include <stdint.h>
#include <microkit.h>
#include <sddf/timer/client.h>
#include <sddf/util/printf.h>

/* Configuration */
#define TIMER_CH_ID 1
#define PARTITION_CHANNEL_START 2
#define P1_SPD_CH_ID 2
#define P2_SPD_CH_ID 3
#define P3_SPD_CH_ID 4

#define NUM_PARTITIONS 3

/* Partition specific */
#define P1_LEN 100 * NS_IN_MS
#define P2_LEN 100 * NS_IN_MS
#define P3_LEN 100 * NS_IN_MS

PARTITION_SHARED_t *p1_state;
PARTITION_SHARED_t *p2_state;
PARTITION_SHARED_t *p3_state;

PARTITION_ATTR_t *p1_attr;
PARTITION_ATTR_t *p2_attr;
PARTITION_ATTR_t *p3_attr;

PARTITION_SHARED_t *partition_state_list[NUM_PARTITIONS] = {0};

/* Partition scheduling management */
PARTITION_ATTR_t *partition_info_list[NUM_PARTITIONS] = {0};
uint64_t plist_head = 0;

int init_finished = 0;

uint64_t get_next_partition_idx(void) {
    return plist_head++ % NUM_PARTITIONS;
}

/* Current time allocated for initialisation timeout */
uint64_t curr_init = 0;
/* Amount to decrement every time initialisation isn't complete */
uint64_t init_decrement = 5;

void setup_partition_config(void) {
    /* Assign partitions to list */
    partition_state_list[0] = p1_state;
    partition_state_list[1] = p2_state;
    partition_state_list[2] = p3_state;

    /* Set partitions states / info */
    for (int i = 0; i < NUM_PARTITIONS; ++i) {
        set_partition_state(partition_state_list[i], INIT);
    }

    p1_attr->LENGTH = P1_LEN;
    p1_attr->RELATIVE_START = 0;
    p2_attr->LENGTH = P2_LEN;
    p2_attr->RELATIVE_START = P1_LEN;
    p3_attr->LENGTH = P3_LEN;
    p3_attr->RELATIVE_START = P2_LEN;

    partition_info_list[0] = p1_attr;
    partition_info_list[1] = p2_attr;
    partition_info_list[2] = p3_attr;
}

void init(void) {
    
    setup_partition_config();

    /* Set init completion time */
    /* Assume that the optimistic init time is the sum of all partitions timeslice */
    for (int i = 0; i < NUM_PARTITIONS; ++i) {
        curr_init += partition_info_list[i]->LENGTH;
    }

    /* Register timer interrupt for init */
    
    sddf_timer_set_timeout(TIMER_CH_ID, curr_init); 
}

void notified(microkit_channel ch)
{
    switch (ch) {
    case TIMER_CH_ID:

        if (!init_finished) {
            /* Check state of partitions */
            for (int i = 0; i < NUM_PARTITIONS; ++i) {
                /* If not all ready, wait for init */
                if (partition_state_list[i]->state != READY) {
                    microkit_dbg_puts("INITIALISATION NOT COMPLETE. WAITING FOR INITIALISATION... \n");
                    
                    if (curr_init - init_decrement > curr_init) {
                        microkit_dbg_puts("Overflow init time?\n");
                    }
                    /* Decrease initialisation time by some fixed increment */
                    curr_init = curr_init - init_decrement;
                    sddf_timer_set_timeout(TIMER_CH_ID, curr_init);
                    return;
                }
            }
            /* Set flag once finished */
            init_finished = 1;
        }

        /* Normal operation (init complete )*/
        int next_partition = get_next_partition_idx();
        partition_state_list[next_partition]->state = RUNNING; 
        /* Notify partition channels */
        microkit_notify(next_partition + PARTITION_CHANNEL_START);
        sddf_timer_set_timeout(TIMER_CH_ID, partition_info_list[next_partition]->LENGTH);
        break;

    default:
        microkit_dbg_puts("TIMER|ERROR: unexpected channel!\n");
    }

}