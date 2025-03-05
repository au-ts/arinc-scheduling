#include "p3.h"

void p3_initialize(void) {
  // add initialization code here
  printf("P3: INIT USER APPLICATION\n");
}

void p3_timeTriggered(void) {
  // add compute phase code here
  int private_val;
  while ((private_val = read_p2()) >= 0) {
    printf_("P3: Received message from P2!: %d\n", private_val);
  }
  printf("\n");
  int broadcast_val = read_broadcast();
  if (broadcast_val > 0) {
    printf("P3: Received broadcast from P1: %d\n", broadcast_val);
  }

  /* If no valid message, return */
}
