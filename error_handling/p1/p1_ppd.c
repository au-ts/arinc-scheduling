#include "p1.h"

#define SPD_CH_ID 5

volatile SAMPLING_PORT_TYPE *P2_PORT;
volatile SAMPLING_PORT_TYPE *BROADCAST_PORT;

volatile PD_STATUS_t *STATUS;

void p1_initialize(void);
void p1_timeTriggered(void);
microkit_msginfo handle_error(void);
void handle_error_2(void);

void send_p2(int message) {
    write_to_port(P2_PORT, message);
}

void broadcast(int message) {
    write_to_port(BROADCAST_PORT, message);
}


void init(void) {
    microkit_dbg_puts("Initialising P1 PPD\n");
    p1_initialize();
    STATUS->handle_error_fn = &handle_error;
    STATUS->status = READY;
}

uint64_t saved_lr;

microkit_msginfo msg_info;

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    // __asm__ volatile (
    //     "mov %[saved], x30\n\t"  
    //     : [saved] "=r" (saved_lr)
    //     :
    //     : "memory"
    // );
    // microkit_dbg_puts("P1 PPD\n");
    switch (channel) {
        case SPD_CH_ID:    

            __asm__ volatile (
                "mov %[saved], x30\n\t"  
                : [saved] "=r" (saved_lr)
                :
                : "memory"
            );

            /* Run periodic application code */
            msg_info = msginfo;
            while(1) {

            };
            // p1_timeTriggered();

            break;
        
        default:
        microkit_dbg_puts("ERROR!\n");
            break;
    }
    // sddf_dprintf("bye\n"); 
    // sddf_dprintf("hello\n");
    return microkit_msginfo_new(0, 0);
}

void notified(microkit_channel ch) {};

void handle_error_2(void) {
    __asm__ volatile (
        "mov x30, %[saved]\n\t"  
        :
        : [saved] "r" (saved_lr)
        : "x30"
    );

    __asm__ volatile (
        "ret\n\t"        
    );
}

microkit_msginfo handle_error(void) {
    sddf_dprintf("IN HANDLE ERROR P1 PPD\n"); 
    // __asm__ volatile (
    //     "mov x30, %[saved]\n\t"  
    //     :
    //     : [saved] "r" (saved_lr)
    //     : "x30"
    // );

    return microkit_msginfo_new(0, 0);
}