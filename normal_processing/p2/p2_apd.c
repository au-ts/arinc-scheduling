#include "p2.h"

#define SPD_CH_ID 8

/* Send Port */
volatile SAMPLING_PORT_TYPE *P3_APD_PORT;
/* Receive Port */
// volatile SAMPLING_PORT_TYPE *P1_RECV;
volatile SAMPLING_PORT_TYPE *P1_APD_RECV;
// volatile SAMPLING_PORT_TYPE *P1_BROADCAST_RECV;

volatile PD_STATUS_t *STATUS;

void p2_initialize(void);
void p2_timeTriggered(void);
void handle_error(void);

void send_p3(int message) {
    write_to_port(P3_PORT, message);
}

int read_p1_apd(void) {
    return read_from_port(P1_APD_RECV);
}

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
            sddf_dprintf("P1 APD: %d\n", read_p1_apd());
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
    send_p3(i);
    i = 0;
    loop = 0;

    sddf_dprintf("APD HANDLING ERROR!\n");

    STATUS->status = READY;
}