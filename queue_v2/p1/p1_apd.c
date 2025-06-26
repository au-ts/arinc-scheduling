#include "p1.h"

#define SPD_CH_ID 8

// volatile SAMPLING_PORT_TYPE *P2_PORT;
volatile SAMPLING_PORT_TYPE *P1_APD_PORT;
// volatile SAMPLING_PORT_TYPE *BROADCAST_PORT;

volatile PD_STATUS_t *STATUS;

void p1_initialize(void);
void p1_timeTriggered(void);
void handle_error(void);

void send_p2(int message) {
    write_to_port(P1_APD_PORT, message);
}

// void broadcast(int message) {
//     write_to_port(BROADCAST_PORT, message);
// }

int i = 0;
int loop = 0;

void init(void) {
    microkit_dbg_puts("Initialising P1 aPD\n");
    STATUS->status = READY;
    STATUS->handle_error_fn = &handle_error;
    // sddf_dprintf("APD PC: %p\n", STATUS->handle_error_fn);
}

void notified(microkit_channel ch) {
    switch (ch) {
        case SPD_CH_ID: {   
            microkit_dbg_puts("aPD\n");
            loop = 1;
            /* Run periodic application code */
            while (loop) {
                ++i;
            }
            // break;
        }
    }
};

void handle_error(void) {
    send_p2(i);
    i = 0;
    loop = 0;

    sddf_dprintf("APD HANDLING ERROR!\n");

    STATUS->status = READY;
}