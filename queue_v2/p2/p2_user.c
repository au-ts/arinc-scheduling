#include "p2.h"

void p2_initialize(void) {
  // add initialization code here
  printf("P2: INIT USER APPLICATION\n");
}

void p2_timeTriggered(void) {
  // add compute phase code here
  int privateval = read_p1();
  if (privateval % 2 == 0) {
    send_p3(privateval);
    sddf_dprintf("P2: Sending even number : %d\n", privateval);
  } else {
    sddf_dprintf("P2: Blocked odd number : %d\n", privateval);
  }

  sddf_dprintf("P1 APD: %d\n", read_p1_apd());
}
