#include "p1.h"

int value = 0;
int broadcast_val = 1000;

void p1_initialize(void) {
  // add initialization code here
  sddf_dprintf("Setting initial value: %d\n", value);
  send_p2(value);
  sddf_dprintf("Setting initial broadcast: %d\n", broadcast_val);
  broadcast(broadcast_val); 
}

void p1_timeTriggered(void) {
  // add compute phase code here
  // sddf_dprintf("In p1_time triggered\n");
  // while(1) {
  //   ++value;
  // }
  ++value;
  --broadcast_val;
  send_p2(value);
  broadcast(broadcast_val);
}
