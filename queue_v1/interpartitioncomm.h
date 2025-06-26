#pragma once
#include "port.h"
#include "printf.h"
#include <string.h>
#include <microkit.h>

#define ERR_BUF_FULL -1
#define ERR_BUF_EMPTY -1
#define ERR_MSG_INVALID -2

void check_set_message(SAMPLING_PORT_TYPE *from, SAMPLING_PORT_TYPE *to);
void reset_sampling_port(SAMPLING_PORT_TYPE *port);
void write_sampling_message(SAMPLING_PORT_TYPE *port, int message); 

void init_queuing_port(QUEUING_PORT_TYPE *port);
/* Return -1 if port not valid or empty, otherwise returns value from port >=0 */
int read_sampling_message(SAMPLING_PORT_TYPE *port);
/* Return -1 on port full */
int send_queuing_message(QUEUING_PORT_TYPE *port, int message);
/* Return ERR_BUF_FULL on port full, ERR_MSG_INVALID on message invalid */
int receive_queuing_message(QUEUING_PORT_TYPE *port);

void transfer_queuing_buffers(QUEUING_PORT_TYPE *from, QUEUING_PORT_TYPE *to);