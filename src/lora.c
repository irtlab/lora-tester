#include "lora.h"
#include "voltage.h"
#include <twr.h>

static twr_cmwx1zzabz_t modem;

lora_state_t lora;


static void lora_callback(twr_cmwx1zzabz_t* self, twr_cmwx1zzabz_event_t event, void* param)
{
    switch(event) {
        case TWR_CMWX1ZZABZ_EVENT_ERROR:
            twr_log_error("lora: Modem reported error");
            break;

        case TWR_CMWX1ZZABZ_EVENT_CUSTOM_AT:
            twr_log_debug("lora: custom AT");
            break;

        case TWR_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START:
            twr_log_debug("lora: Sending message");
            voltage_measure(20, false);
            
            break;

        case TWR_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE:
            twr_log_debug("lora: Message sent");
            break;

        case TWR_CMWX1ZZABZ_EVENT_READY:
            twr_log_debug("lora: Modem is ready");
            break;

        case TWR_CMWX1ZZABZ_EVENT_JOIN_SUCCESS:
            twr_log_info("lora: JOIN succeeded");
            twr_cmwx1zzabz_rfq(&modem);
            break;

        case TWR_CMWX1ZZABZ_EVENT_JOIN_ERROR:
            twr_log_error("lora: JOIN failed");
            break;

        case TWR_CMWX1ZZABZ_EVENT_CONFIG_SAVE_DONE:
            twr_log_debug("lora: Modem configuration saved");
            break;

        case TWR_CMWX1ZZABZ_EVENT_MESSAGE_RECEIVED:
            twr_log_info("lora: Message received");
            break;

        case TWR_CMWX1ZZABZ_EVENT_MESSAGE_RETRANSMISSION:
            twr_log_info("lora: Retransmitting message");
            voltage_measure(20, false);
            break;

        case TWR_CMWX1ZZABZ_EVENT_MESSAGE_CONFIRMED:
			lora.ack_received = true;
            twr_log_debug("lora: Message confirmed");
            twr_cmwx1zzabz_rfq(&modem);
            twr_scheduler_plan_now(0);
            break;

        case TWR_CMWX1ZZABZ_EVENT_MESSAGE_NOT_CONFIRMED:
			lora.ack_received = false;
			lora.rssi = 0;
            lora.snr = 0;
            twr_log_warning("lora: Message NOT confirmed");
            twr_scheduler_plan_now(0);
            break;

        case TWR_CMWX1ZZABZ_EVENT_RFQ:
            twr_cmwx1zzabz_get_rfq(&modem, &lora.rssi, &lora.snr);
            twr_log_debug("lora: RSSI=%ld, SNR=%ld", lora.rssi, lora.snr);
            twr_cmwx1zzabz_frame_counter(&modem);
            break;

        case TWR_CMWX1ZZABZ_EVENT_LINK_CHECK_OK:
            twr_cmwx1zzabz_get_link_check(&modem, &lora.margin, &lora.gwcount);
            twr_log_debug("lora: margin=%d, gwcount=%d", lora.margin, lora.gwcount);
            twr_cmwx1zzabz_rfq(&modem);
            break;

        case TWR_CMWX1ZZABZ_EVENT_LINK_CHECK_NOK:
            twr_atci_printfln("lora: Link check failed");
            break;

        case TWR_CMWX1ZZABZ_EVENT_MODEM_FACTORY_RESET:
            twr_log_debug("lora: module factory reset");
            break;

        case TWR_CMWX1ZZABZ_EVENT_FRAME_COUNTER:
            twr_cmwx1zzabz_get_frame_counter(&modem, &lora.uplink, &lora.downlink);
            twr_log_debug("lora: uplink=%lu, downlink=%lu", lora.uplink, lora.downlink);
            break;

        default:
            break;
    }
}


void lora_init(bool debug)
{
    twr_log_debug("lora: Initializing LoRa modem");
    // Initialize LoRa modem
    twr_cmwx1zzabz_init(&modem, TWR_UART_UART1);
    twr_cmwx1zzabz_set_event_handler(&modem, lora_callback, NULL);
    twr_cmwx1zzabz_set_class(&modem, TWR_CMWX1ZZABZ_CONFIG_CLASS_A);
    twr_cmwx1zzabz_set_debug(&modem, debug);

    // Initialize AT command interface. This essentially just stores a reference
    // to the modem in an internal variable so that AT commands can access it.
    twr_at_lora_init(&modem);
}


void lora_send(const void* msg, size_t len, bool confirmed)
{
    if (confirmed) twr_cmwx1zzabz_send_message_confirmed(&modem, msg, len);
    else twr_cmwx1zzabz_send_message(&modem, msg, len);
    lora.confirmed = confirmed;
}


bool lora_ready()
{
    static bool ready = false;

    if (!ready) {
        if (!twr_cmwx1zzabz_is_ready(&modem)) {
            twr_scheduler_plan_current_relative(100);
            return false;
        }
        ready = true;
    }
    return true;
}
