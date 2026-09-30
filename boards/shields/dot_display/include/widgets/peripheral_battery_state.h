#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

#include <widgets.h>

struct zmk_peripheral_battery_state_ui
{
	lv_obj_t *box;
	lv_obj_t *txt;
	lv_obj_t *sym;
};

struct peripheral_battery_state
{
	uint8_t source;
	uint8_t status;
};

struct peripheral_conn_state
{
	uint8_t source;
	bool    conn;
};

void                            peripheral_battery_state_cb(struct peripheral_battery_state);
struct peripheral_battery_state peripheral_battery_get_state(const zmk_event_t *);
void                            peripheral_conn_state_cb(struct peripheral_conn_state);
struct peripheral_conn_state    peripheral_conn_get_state(const zmk_event_t *);
void                            zmk_peripheral_battery_state_init(struct zmk_widget *, lv_obj_t *);
