#include "p3.h"
#include "p3_config.h"

// int read_p2(QUEUE_MSG *output_buf, process_internal *caller, int num_retries) {
//   return receive_queuing_message(P2_RECV, output_buf, caller, num_retries);
// }

// int read_broadcast(void) {
//   return read_sampling_message(P1_BROADCAST_RECV);
// }

void periodic_init(void) {
  // add initialization code here
  printf("P3: init pCo\n");
}

void aperiodic_init(void) {
  printf("P3: init aCo\n");

}

void periodic(void) {
  // add compute phase code here
  // int private_val;
  // while ((private_val = read_p2()) >= 0) {
  //   printf_("P3: Received message from P2!: %d\n", private_val);
  // }
  // printf("\n");
  // int broadcast_val = read_broadcast();
  // if (broadcast_val > 0) {
  //   printf("P3: Received broadcast from P1: %d\n", broadcast_val);
  // }

  /* If no valid message, return */
}

void aperiodic(void) {
  // Something
}

void periodic_recovery(void) {
  printf("P3: In user recovery for periodic thread\n");
}
