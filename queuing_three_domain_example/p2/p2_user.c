#include "p2.h"

static int application_buf[QUEUING_BUFFER_LEN] = {0};

void p2_initialize(void) {
  // add initialization code here
  printf("P2: INIT USER APPLICATION\n");
}

void p2_timeTriggered(void) {
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
