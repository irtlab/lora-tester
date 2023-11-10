#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    int32_t rssi;
    int32_t snr;
    uint8_t margin;
    uint8_t gwcount;
    uint32_t uplink;
    uint32_t downlink;
	bool ack_received;
    bool confirmed;
} lora_state_t;

extern lora_state_t lora;

extern void lora_init(bool debug);
extern void lora_send(const void* msg, size_t len, bool confirmed);
extern bool lora_ready();
