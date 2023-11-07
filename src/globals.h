#pragma once
#include "interval.h"

#define MEASURE_INTERVAL (15 * SECOND)
#define SEND_INTERVAL    (1 * MINUTE)

#define FLAG_UPLINK_CONFIRMED   0x01  // 00000001 in binary for message_t.flags
#define FLAG_ACK_RECEIVED       0x02  // 00000010 


extern int measurements_left;
