#pragma once
#include <twr.h>

extern float voltage_min;

extern void voltage_measure(twr_tick_t offset, bool wakeup);

extern void voltage_init();
