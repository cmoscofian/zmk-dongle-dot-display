#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>

#include <widgets.h>

struct zmk_wpm_state_ui
{
	lv_obj_t *txt;
};

struct wpm_state
{
	uint8_t wpm;
};

void             wpm_state_cb(struct wpm_state);
struct wpm_state wpm_get_state(const zmk_event_t *eh);
void             zmk_wpm_state_init(struct zmk_widget *widget, lv_obj_t *parent);
