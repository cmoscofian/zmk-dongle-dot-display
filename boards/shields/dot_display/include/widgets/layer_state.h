#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

#include <widgets.h>

struct zmk_layer_state_ui
{
	lv_obj_t *txt;
};

struct layer_state
{
	const char *label;
	uint8_t     layer;
};

void               layer_state_cb(struct layer_state);
struct layer_state layer_get_state(const zmk_event_t *eh);
void               zmk_layer_state_init(struct zmk_widget *widget, lv_obj_t *parent);
