#include "led.h"
#include <twr.h>

static twr_led_t led;

void led_init()
{
    twr_log_debug("led: Initializing Core module LED");
    twr_led_init(&led, TWR_GPIO_LED, false, false);
}


void led_set(bool onoff)
{
    twr_led_set_mode(&led, onoff ? TWR_LED_MODE_ON : TWR_LED_MODE_OFF);
}


bool led_at_set(twr_atci_param_t *param)
{
    if (param->length != 1) return false;

    if (param->txt[0] == '1') {
        led_set(true);
        return true;
    }

    if (param->txt[0] == '0') {
        led_set(false);
        return true;
    }

    return false;
}
