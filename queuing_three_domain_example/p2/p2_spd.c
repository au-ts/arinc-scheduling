#include "p2.h"
#include "p2_config.h"
#include "spd_shared.h"
#include <sddf/timer/client.h>

/* PORTS */
SAMPLING_PORT_TYPE *SEND_P2_PORT;
SAMPLING_PORT_TYPE *SEND_ALL_PORT;

/* PARTITION INFO */

partition_internal *partition_state;
/* TODO map memory for aco_status/pco_status */
process_internal *upd_status;

/* TODO map memory for regs */
seL4_UserContext *aco_ctxt;

seL4_UserContext *pco_ctxt;

int aco_flag = 0;

uintptr_t uart_base;

#define UART_REG(x) ((volatile uint32_t *)(uart_base + (x)))
// #define UART_BASE 0x10004000
#define UART_WFIFO 0x0
#define UART_STATUS 0xC
#define UART_TX_FULL (1 << 21)

static void uart_init() {}

static void putc(uint8_t ch)
{
    while ((*UART_REG(UART_STATUS) & UART_TX_FULL));
    *UART_REG(UART_WFIFO) = ch;
}

static void putstr(const char *s)
{
    while (*s) {
        putc((uint8_t)*s++);
    }
}

static void print_cycle(ccnt_t s, ccnt_t e, char flag) {
    // printf("NUMBER CYCLES = %d\n", e - s);
    char buf[128];
    snprintf(buf, sizeof(buf), "%c :Number cycles = %llu\n", flag, (unsigned long long) (e - s));
    putstr(buf);
}

ccnt_t start;
ccnt_t end;
ccnt_t upd_s;
ccnt_t upd_notify;


void init(void) { 
    /* sPD setup */

    printf("P2 SPD INIT\n");
};

void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
        case SCHEDULER_CH_ID: {
            microkit_dbg_puts("In P2 sPD\n");
            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(start);
            #endif

            /* Check if partition overrun, and signal ePD instead */
            if (upd_status->pco_status == RUNNING || upd_status->pco_status == RECOVER) {
                microkit_dbg_puts("P2 overrun!!!!!\n");
                microkit_notify(EPD_CH_ID);
                break;
            }

            microkit_dbg_puts("0\n");
            /* Timeout for partition setup */
            sddf_timer_set_timeout(TIMER_CH_ID, PARTITION_SETUP_TIME);

            /* Interpartition Communication Semantics */
    
            microkit_dbg_puts("1\n");
            printf("acostatus: %d\n", upd_status->aco_status);
            /* If the aCo running (normal operation after first-run), cleanup uPD */
            if (upd_status->aco_status == RUNNING) {
                microkit_dbg_puts("Handling aCo P2\n");
                
                /* Save registers of the uPD */
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + 2, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); // Save current PC of Client

                printf("ACO SAVED PC from read: 0x%x\n", aco_ctxt->pc);

                /* Resume uPD from aCo handling fn */
                microkit_pd_restart(2, (seL4_Word) upd_status->aco_recovery_fn);

                /* Resume the suspended uPD */
                seL4_TCB_Resume(BASE_TCB_CAP + 2);
            }
            microkit_dbg_puts("2\n");

            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(end);
            #endif
            
            break;
        }

        case TIMER_CH_ID: {
        /* Partition setup complete */ 
            // seL4_DebugDumpScheduler();
            microkit_dbg_puts("Partition setup finished P2\n");
            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(upd_s);
            #endif
            partition_state->state = RUNNING;

            microkit_dbg_puts("4\n");
            if (upd_status->aco_status != READY) {
                microkit_dbg_puts("ERROR!: aCo did not finish handling!\n");
                break;
            } 
            microkit_dbg_puts("5\n");
            /* Notify uPD's root co-thread */
            seL4_DebugDumpScheduler();
            microkit_notify(UPD_CH_ID);
            microkit_dbg_puts("6\n");

            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(upd_notify);
            print_cycle(start, end, 'C');
            print_cycle(upd_s, upd_notify, 'D');
            #endif
            break;
        }

        case UPD_CH_ID: {
            /* If not enough time remaining to restore registers, and yield, set flag and skip */
            // if (!(sddf_timer_time_now(TIMER_CH_ID))) {
            //     aco_flag = 1;
            //     break;
            // }
            microkit_dbg_puts("Restoring aCo registers from spd\n");

            // seL4_TCB_Suspend(BASE_TCB_CAP + 2);
            /* Restore aCo registers */
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + 2, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); 
            printf("ACO SAVED PC: 0x%x\n", aco_ctxt->pc);
            microkit_dbg_puts("After restoring\n");
            /* Reset saved aCo context */
            memset(aco_ctxt, 0, sizeof(*aco_ctxt));
            upd_status->aco_status = RUNNING;
            // seL4_TCB_Resume(BASE_TCB_CAP + 2);

            break;
        }

        case UPD_PCO_INIT_CH: {
            /* pCo has finished its init. Save its context */
            microkit_dbg_puts("sPD 2: Saving pCo registers\n");
            seL4_TCB_ReadRegisters(BASE_TCB_CAP + 2, seL4_False, 0, NUM_REG_SAVE, pco_ctxt);
            break;
        } 

        case UPD_FINISH_INIT_CH: {
            
            partition_state->state = READY;
            partition_state->upd_state = READY;
            microkit_dbg_puts("FINISH INIT UPD 2\n");
            break;
        }

        default: {
            microkit_dbg_puts("ERROR sPD 2: invalid channel\n");
        }
    }
};


