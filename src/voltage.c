#include "voltage.h"
#include <math.h>
#include <twr.h>
#include "globals.h"

float voltage_min = NAN;
float voltage_current = NAN;
static bool wakeup;

static twr_scheduler_task_id_t task_id;


static void update_min_voltage(float voltage) {
    if (isnan(voltage)) voltage_min = NAN;
    if (voltage < voltage_min || isnan(voltage_min)) voltage_min = voltage;
}


static void voltage_task(void* param)
{
    // Returns true if there is another active battery measurement task already
    // in progress.
    if (!twr_module_battery_measure()) {
        twr_scheduler_plan_current_now();
        return;
    }
}


void voltage_measure(twr_tick_t offset, bool w)
{
    wakeup = w;
    if (offset == 0) {
        twr_log_debug("voltage: Measuring battery voltage");
        twr_module_battery_measure();
    } else {
        twr_log_debug("voltage: Scheduling battery voltage measurement in %d ms", (int)offset);
        twr_scheduler_plan_relative(task_id, offset);
    }
}


static void event_handler(twr_module_battery_event_t event, void* param)
{
    switch(event) {
        case TWR_MODULE_BATTERY_EVENT_UPDATE:
            twr_module_battery_get_voltage(&voltage_current);
            break;

        case TWR_MODULE_BATTERY_EVENT_LEVEL_LOW:
            twr_log_info("voltage: Battery reports low voltage");
            twr_module_battery_get_voltage(&voltage_current);
            break;

        case TWR_MODULE_BATTERY_EVENT_LEVEL_CRITICAL:
            twr_log_warning("voltage: Battery reports critical voltage");
            twr_module_battery_get_voltage(&voltage_current);
            break;

        case TWR_MODULE_BATTERY_EVENT_ERROR:
            twr_log_error("voltage: Battery voltage measurement error");
            voltage_current = NAN;
            break;

        default:
            break;
    }

    measurements_left--;
    update_min_voltage(voltage_current);
    twr_log_debug("voltage: current=%.2f V, min=%.2f V", voltage_current, voltage_min);

    if (wakeup) {
        // Wake up the main application task
        twr_scheduler_plan_now(0);
    }
}


void voltage_init()
{
    twr_log_debug("voltage: Initializing battery voltage measurements");
    twr_module_battery_init();
    twr_module_battery_set_event_handler(event_handler, NULL);
    task_id = twr_scheduler_register(voltage_task, NULL, TWR_TICK_INFINITY);
}
