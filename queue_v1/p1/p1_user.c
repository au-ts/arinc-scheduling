#include "p1.h"

int value = 0;
int broadcast_val = 1000;

void p1_initialize(void) {
  // add initialization code here
  printf("Setting initial value: %d\n", value);
  send_p2(value);
  printf("Setting initial broadcast: %d\n", broadcast_val);
  broadcast(broadcast_val); 
}

void p1_timeTriggered(void) {
  // add compute phase code here
  ++value;
  --broadcast_val;
  send_p2(value);
  broadcast(broadcast_val);
}
