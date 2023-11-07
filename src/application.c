#include <stdarg.h>
#include <twr.h>
#include "voltage.h"
#include "lora.h"
#include "globals.h"
#include "temp.h"
#include "led.h"
#include "button.h"


typedef struct __attribute__((packed)) message {
    int16_t temperature;
    uint8_t min_voltage;
    int16_t rssi;
    int8_t snr;
    uint8_t flags;
} message_t;


static enum state {
    STATE_IDLE,
    STATE_START_MEASUREMENTS,
    STATE_MEASURING,
    STATE_TRANSMITTING
} state = STATE_IDLE;


static volatile twr_tick_t tick = 0;
static bool send;


static inline uint16_t htons(uint16_t v)
{
    return v >> 8 | (v << 8);
}


static message_t build_status_message()
{
    static message_t msg;
    float t = temp_get();

    msg.temperature = isnan(t) ? INT16_MAX : (int16_t)round((t * 10.0));
    msg.min_voltage = isnan(voltage_min) ? UINT8_MAX : (uint8_t)round(voltage_min * 10.0);
    msg.rssi = lora.rssi;
    msg.snr = lora.snr;

    // Flag set logic.
    
    //msg.flags = 0; // Reset (DEBUG LINE) 
    
    if (last_uplink_confirmed) {
        msg.flags |= FLAG_UPLINK_CONFIRMED;
    }
    if (lora.ack_received) {
        msg.flags |= FLAG_ACK_RECEIVED;
    }


    return msg;
}


static bool at_get_status()
{
    if (state != STATE_IDLE) return false;
    state = STATE_START_MEASUREMENTS;
    send = false;
    twr_scheduler_plan_now(0);
    return true;
}


static bool at_send()
{
    if (state != STATE_IDLE) return false;
    state = STATE_START_MEASUREMENTS;
    send = true;
    twr_scheduler_plan_now(0);
    return true;
}


#if defined(VERSION) && defined(BUILD_DATE)
static bool at_version()
{
    twr_atci_printfln("$VER: %s built on %s", VERSION, BUILD_DATE);
    return true;
}
#endif


void on_click(void)
{
    at_send();
}


void application_init(void)
{
    int log_level = TWR_LOG_LEVEL_DUMP;
    bool debug = true;

#ifdef RELEASE
    log_level = TWR_LOG_LEVEL_INFO;
    debug = false;
#endif

    twr_log_init(log_level, TWR_LOG_TIMESTAMP_ABS);
    lora_init(debug);

    voltage_init();
    temp_init();
    led_init();
    button_init(on_click);

    static const twr_atci_command_t commands[] = {
        TWR_AT_LORA_COMMANDS,
        TWR_ATCI_COMMAND_CLAC,
        TWR_ATCI_COMMAND_HELP,
        LED_AT_COMMANDS,
        {"$SEND", at_send, NULL, NULL, NULL, "Send current status immediately"},
        {"$STATUS", at_get_status, NULL, NULL, NULL, "Get current device status"},
        #if defined(VERSION) && defined(BUILD_DATE)
        {"$VER", at_version, NULL, NULL, NULL, "Get firmware version and build date"}
        #endif
    };
    twr_atci_init(commands, TWR_ATCI_COMMANDS_LENGTH(commands));
}


void application_task(void)
{
    tick = tick + 1;
    if (!lora_ready()) return;

    switch(state) {
    case STATE_IDLE:
        break;

    case STATE_START_MEASUREMENTS:
        state = STATE_MEASURING;

        measurements_left=1;
        temp_measure();

        measurements_left++;
        voltage_measure(0, true);
        break;

    case STATE_MEASURING:
        if (measurements_left > 0) break;
        if (send) {
            message_t msg = build_status_message();
            lora_send(&msg, sizeof(msg), true);
            state = STATE_TRANSMITTING;
            led_set(true);
        } else {
            twr_atci_printfln("status: temperature=%.1f C, min_voltage=%.1f V",
                temp_get(), voltage_min);
            state = STATE_IDLE;
        }

        break;

    case STATE_TRANSMITTING:
        led_set(false);
        state = STATE_IDLE;
        break;

    default:
        twr_log_error("Unknown application state %d", state);
        break;
    }
}
