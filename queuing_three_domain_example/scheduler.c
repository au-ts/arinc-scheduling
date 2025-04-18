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

#define BASE_PARTITION_EPD_CHANNEL 32
#define P1_EPD_CH_ID 32
#define P1_EPD_CH_ID 33
#define P1_EPD_CH_ID 34

#define NUM_PARTITIONS 3

#define EPD_TIMESLICE 50

#define POST_RECOVERY_PADDING = 10 * NS_IN_MS

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

/* Error partitions */
PARTITION_ATTR_t *partition_info_list[NUM_PARTITIONS] = {0};
uint64_t plist_head = 0;

int init_finished = 0;

int first_run = 1;

uint64_t get_next_partition_idx(void) {
    // Error handling partition in between
    return plist_head++ % (NUM_PARTITIONS * 2);
}

uint64_t get_prev_partition_idx(void) { 
    uint64_t curr_head = plist_head;
    return --curr_head;
}

/* Get's previous non-error partition idx */
uint64_t get_prev_normal_partition_idx(void) {
    uint64_t curr_head = plist_head;
    if (curr_head > 0) {
        if (curr_head % 2 == 0) {
            return curr_head - 2;
        } else {
            return curr_head - 1;
        }
    } else {
        /* Return the last non error partition */
        return (NUM_PARTITIONS - 1) * 2;
    }
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
        int partition_to_run = get_next_partition_idx();
        /* Error handling follows normal partition*/
        if (partition_to_run % 2 != 0) {
            /* Partition Error PD's are odd offsets starting from 1 .. 2n + 1 where n is number of partitions */
            microkit_notify(BASE_PARTITION_EPD_CHANNEL + ((partition_to_run - 1) / 2));
            sddf_timer_set_timeout(TIMER_CH_ID, EPD_TIMESLICE);
            break;

        } else { 
            /* Notify ePD to bind/unbind from pPD */
            if (first_run) {
                first_run = 0;
            } else {
                /* Invariant here is that prev_partition_epd_idx is always odd, and the current partition to run has even idx */
                int prev_partition_epd_idx = get_prev_partition_idx();
                if ((partition_state_list[get_prev_normal_partition_idx()]->recovering_pd & RECOVERY_MASK) != PPD_RECOVERING) {
                    /* PPD is not recovering: no need to unbind/rebind, just run padding */
                    sddf_timer_set_timeout(TIMER_CH_ID, POST_RECOVERY_PADDING);
                } else {
                    /* TODO: calling this instruction would have taken some time, pad for that time too? */
                    microkit_notify(BASE_PARTITION_EPD_CHANNEL + prev_partition_epd_idx);
                    sddf_timer_set_timeout(TIMER_CH_ID, POST_RECOVERY_PADDING)
                }                
            }
            /* Normal partition PD's are even offsets starting from 0 .. 2n where n is number of partitions */
            partition_state_list[(partition_to_run / 2)]->state = RUNNING; 
            /* Notify partition channels */
            microkit_notify(PARTITION_CHANNEL_START + partition_to_run);
            sddf_timer_set_timeout(TIMER_CH_ID, partition_info_list[partition_to_run]->LENGTH);
            break;
        }

    default:
        microkit_dbg_puts("TIMER|ERROR: unexpected channel!\n");
    }

}