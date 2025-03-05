#pragma once
#include "port.h"

void check_set_message(volatile SAMPLING_PORT_TYPE *from, volatile SAMPLING_PORT_TYPE *to);
void reset_port(volatile SAMPLING_PORT_TYPE *port);
void write_to_port(volatile SAMPLING_PORT_TYPE *port, int message); 

/* Return -1 if port not valid or empty, otherwise returns value from port >=0 */
int read_from_port(volatile SAMPLING_PORT_TYPE *port);