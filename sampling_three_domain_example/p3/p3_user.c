#include "p3.h"

int curr_val = 0;

void p3_initialize(void) {
  // add initialization code here
  printf("P3: INIT USER APPLICATION\n");
}

void p3_timeTriggered(void) {
  // add compute phase code here
  int private_val = read_p2();
  int broadcast_val = read_broadcast();

  /* If no valid message, return */
  if (private_val == -1 || broadcast_val == -1) {
    return;
  }

  if (private_val == broadcast_val) {
    printf("Broadcast value : %d == P2 val : %d\n", broadcast_val, private_val);
  } else if (private_val > curr_val) {
    curr_val = private_val;
    printf("P3: Received Value From P2 : %d\n\n", private_val);
  } else {
    printf("P3: Received Broadcast from P1: %d\n\n", broadcast_val);
  }
}
