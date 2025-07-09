#include "p1.h"

int value = 0;
int broadcast_val = 1000;

void p1_pco_initialize(void) {
  // add initialization code here
  printf("P1 PCO INIT\n");
  send_p2(value);
  broadcast(broadcast_val); 
}

void p1_aco_initialize(void) {
  // add initialization code here
  printf("P1 ACO INIT\n");
}

void p1_timeTriggered(void) {
  // add compute phase code here
  ++value;
  --broadcast_val;
  send_p2(value);
  broadcast(broadcast_val);
}
