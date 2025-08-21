#include "p1.h"
#include "p1_config.h"
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


// #define UART_BASE                 0x9000000
// #define PL011_TCR                 0x030
// #define PL011_UARTDR              0x000
// #define PL011_UARTFR              0x018
// #define PL011_UARTFR_TXFF         (1 << 5)
// #define PL011_CR_UART_EN          (1 << 0)
// #define PL011_CR_TX_EN            (1 << 8)

// static void uart_init()
// {
//     /* Enable the device and transmit */
//     *UART_REG(PL011_TCR) |= (PL011_CR_TX_EN | PL011_CR_UART_EN);
// }

// static void putc(uint8_t ch)
// {
//     while ((*UART_REG(PL011_UARTFR) & PL011_UARTFR_TXFF) != 0);
//     *UART_REG(PL011_UARTDR) = ch;
// }


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
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('A');
    // putc('\n');

    printf("P1 SPD INIT\n");
};


void notified(microkit_channel ch) {
    switch (ch) {
    /* Start of partition time slice */
        case SCHEDULER_CH_ID: {

            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(start);
            #endif

            printf("P1 sPD scheduled\n");

            /* Check if partition overrun, and signal ePD instead */
            if (upd_status->pco_status == RUNNING || upd_status->pco_status == RECOVER) {
                microkit_notify(EPD_CH_ID);
                break;
            }

            /* Timeout for partition setup */
            sddf_timer_set_timeout(TIMER_CH_ID, PARTITION_SETUP_TIME);

            /* Interpartition Communication Semantics */
    
            /* If the aCo running (normal operation after first-run), cleanup uPD */
            if (upd_status->aco_status == RUNNING) {
                
                /* Save registers of the uPD */
                seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); // Save current PC of Client

                /* Resume uPD from aCo handling fn */
                microkit_pd_restart(UPD_TCB_ID, (seL4_Word) upd_status->aco_recovery_fn);

                /* Resume the suspended uPD */
                seL4_TCB_Resume(BASE_TCB_CAP + UPD_TCB_ID);
            }

            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(end);
            #endif

            
            
            
            break;
        }

        case TIMER_CH_ID: {
        /* Partition setup complete */ 
            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(upd_s);
            #endif
            printf("P1 Partition setup complete\n");
            partition_state->state = RUNNING;
            if (upd_status->aco_status != READY) {
                printf("ERROR!: aCo did not finish handling!\n");
                break;
            } 
            microkit_notify(UPD_CH_ID);
            #if ENABLE_CCNT
            SEL4BENCH_READ_CCNT(upd_notify);

            print_cycle(start, end, 'A');
            print_cycle(upd_s, upd_notify, 'B');
            #endif
            /* Notify uPD's root co-thread */
            break;
        }

        case UPD_CH_ID: {
            /* If not enough time remaining to restore registers, and yield, set flag and skip */
            // if (!(sddf_timer_time_now(TIMER_CH_ID))) {
            //     aco_flag = 1;
            //     break;
            // }

            /* Restore aCo registers */
            seL4_TCB_WriteRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, aco_ctxt); 
            /* Reset saved aCo context */
            memset(aco_ctxt, 0, sizeof(*aco_ctxt));
            upd_status->aco_status = READY;

            break;
        }

        case UPD_PCO_INIT_CH: {
            /* pCo has finished its init. Save its context */
            microkit_dbg_puts("sPD 1: Saving pCo registers\n");
            seL4_TCB_ReadRegisters(BASE_TCB_CAP + UPD_TCB_ID, seL4_False, 0, NUM_REG_SAVE, pco_ctxt);
            break;
        } 

        case UPD_FINISH_INIT_CH: {
            partition_state->state = READY;
            partition_state->upd_state = READY;
            microkit_dbg_puts("FINISH INIT UPD 1\n");
            break;
        }

        default: {
            microkit_dbg_puts("ERROR sPD 1: invalid channel\n");
        }
    }
};


