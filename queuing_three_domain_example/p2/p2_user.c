#include "p2.h"

extern process_internal *aco_status;

// void send_p3(int message) {
//   send_queuing_message(P3_PORT, message);
// }

// int read_p1(void) {
//   return read_sampling_message(P1_RECV);
// }
// int read_broadcast(void) {
//   return read_sampling_message(P1_BROADCAST_RECV);
// }

static int application_buf[QUEUING_BUFFER_LEN] = {0};

void periodic_init(void) {
  // add initialization code here
  printf("P2: init pCo\n");
}

void aperiodic_init(void) {
  // add initialization code here
  printf("P2: init aCo\n");
}

// void periodic(void) {
//   // add compute phase code here
//   // int privateval = read_p1();
//   // if (privateval && privateval % QUEUING_BUFFER_LEN == 0) {
//   //   application_buf[QUEUING_BUFFER_LEN - 1] = privateval;
//   //   printf("P2 sending buffer containing: { ");
//   //   for (int i = 0; i < QUEUING_BUFFER_LEN; ++i) {
//   //     printf("%d ", application_buf[i]);
//   //     // send_p3(application_buf[i]); 
//   //   }
//   //   printf("}\n");
//   // } else {
//   //   application_buf[(privateval % QUEUING_BUFFER_LEN) - 1] = privateval;
//   // }
// }

// void aperiodic(void) {
//   // something
// }

void periodic(void) {
  microkit_dbg_puts("User periodic P2 \n");
  // add compute phase code here
  // ++value;
  // --broadcast_val;
  // send_p2(value);
  // int sum2 = 0;
  // while (1) {
    // ++sum;
  // }
  for (int i = 0; i < 1000; ++i) {
  }
  // broadcast(broadcast_val);
}

void aperiodic(void) {
  microkit_dbg_puts("User aperiodic P2 \n");
  int sum = 0;
  while (1) {
    ++sum;
    if (sum % 87936 == 0) {
      microkit_dbg_puts("IN APERIODIC LOOP\n");
      sum = 0;
    }
  }
}
void periodic_recovery(void) {
  microkit_dbg_puts("P2: In user recovery for periodic thread\n");
}

