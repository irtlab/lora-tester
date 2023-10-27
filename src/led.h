#pragma once
#include <stdbool.h>
#include <twr.h>

#define LED_AT_COMMANDS {"$LED", NULL, led_at_set, NULL, NULL, "Turn the Core module LED on or off"}

extern void led_init();
extern void led_set(bool onoff);
extern bool led_at_set(twr_atci_param_t *param);
