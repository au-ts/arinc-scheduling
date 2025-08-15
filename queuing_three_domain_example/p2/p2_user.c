#include "p2.h"

extern process_internal *aco_status;

void send_p3(int message) {
  send_queuing_message(P3_PORT, message);
}

int read_p1(void) {
  return read_sampling_message(P1_RECV);
}
int read_broadcast(void) {
  return read_sampling_message(P1_BROADCAST_RECV);
}

static int application_buf[QUEUING_BUFFER_LEN] = {0};

void pco_initialize(void) {
  // add initialization code here
  printf("P2: init pCo\n");
}

void aco_initialize(void) {
  // add initialization code here
  printf("P2: init aCo\n");
}

void periodic(void) {
  // add compute phase code here
  int privateval = read_p1();
  if (privateval && privateval % QUEUING_BUFFER_LEN == 0) {
    application_buf[QUEUING_BUFFER_LEN - 1] = privateval;
    printf("P2 sending buffer containing: { ");
    for (int i = 0; i < QUEUING_BUFFER_LEN; ++i) {
      printf("%d ", application_buf[i]);
      send_p3(application_buf[i]); 
    }
    printf("}\n");
  } else {
    application_buf[(privateval % QUEUING_BUFFER_LEN) - 1] = privateval;
  }
}

void aperiodic(void) {
  // something
}
