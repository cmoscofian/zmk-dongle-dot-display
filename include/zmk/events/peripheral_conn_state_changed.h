#pragma once

#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

enum zmk_peripheral_conn_state
{
	PERIPHERAL_CONN_STATE_OPEN,
	PERIPHERAL_CONN_STATE_CONNECTED,
};

struct zmk_peripheral_conn_state_changed
{
	uint8_t source;
	bool    connected;
};

ZMK_EVENT_DECLARE(zmk_peripheral_conn_state_changed);
