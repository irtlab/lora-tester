#include "button.h"
#include <twr.h>

static twr_button_t button;
static void (*callback)(void);


static void handler(twr_button_t *button, twr_button_event_t event, void *param)
{
    switch(event) {
        case TWR_BUTTON_EVENT_CLICK:
            callback();
            break;

        case TWR_BUTTON_EVENT_HOLD:
            break;

        case TWR_BUTTON_EVENT_PRESS:
            break;

        case TWR_BUTTON_EVENT_RELEASE:
            break;
    }
}


void button_init(void (*on_click)(void))
{
    callback = on_click;
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, 0);
    twr_button_set_event_handler(&button, handler, NULL);
}


bool button_sense()
{
    return false;
}
