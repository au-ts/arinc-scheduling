#include "../partition.h"
#include "../port.h"
#include "../interpartitioncomm.h"
#include "../printf.h"
#include <sddf/util/printf.h>
#include <microkit.h>

void send_p3(int message);

int read_p1();
int read_p1_apd(void);
int read_broadcast();
