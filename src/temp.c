#include "temp.h"
#include "globals.h"
#include <twr.h>


static twr_tmp112_t sensor;
static float current = NAN;


static void event_handler(twr_tmp112_t* self, twr_tmp112_event_t event, void* param)
{
    switch(event) {
        case TWR_TMP112_EVENT_ERROR:
            twr_log_error("temp: Sensor reported error");
            current = NAN;
            break;

        case TWR_TMP112_EVENT_UPDATE:
            twr_tmp112_get_temperature_celsius(self, &current);
            break;

        default:
            break;
    }

    measurements_left--;
    twr_log_debug("temp: cur=%.2f C", current);

    // Wake up the main application task
    twr_scheduler_plan_now(0);
}


float temp_get()
{
    return current;
}


void temp_measure()
{
    twr_log_debug("temp: Measuring temperature");
    twr_tmp112_measure(&sensor);
}


void temp_init()
{
    twr_log_debug("temp: Initializing Core module temperature sensor");
    // Initialize Core module temperature sensor
    twr_tmp112_init(&sensor, TWR_I2C_I2C0, 0x49);
    twr_tmp112_set_event_handler(&sensor, event_handler, NULL);
    twr_tmp112_set_update_interval(&sensor, TWR_TICK_INFINITY);
}
