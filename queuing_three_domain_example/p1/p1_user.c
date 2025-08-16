#include "p1.h"
#include "p1_config.h"
#include <cstdint>

int value = 0;
int broadcast_val = 1000;

uint64_t sum = 0;

void send_p2(int message) {
    write_sampling_message(P2_PORT, message);
}

void broadcast(int message) {
    write_sampling_message(BROADCAST_PORT, message);
}

void pco_initialize(void) {
  // add initialization code here
  printf("P1 PCO INIT\n");
  send_p2(value);
  broadcast(broadcast_val); 
}

void aco_initialize(void) {
  // add initialization code here
  printf("P1 ACO INIT\n");
}

void periodic(void) {
  // add compute phase code here
  ++value;
  --broadcast_val;
  send_p2(value);
  broadcast(broadcast_val);
}

void aperiodic(void) {
  while (1) {
    ++sum;
  }
}

void periodic_init(void) {
}

void aperiodic_init(void) {}
