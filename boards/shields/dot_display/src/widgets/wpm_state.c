#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>

#include <fonts.h>
#include <widgets.h>
#include <widgets/wpm_state.h>

LOG_MODULE_REGISTER(wpm_state, LOG_LEVEL_INF);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static struct zmk_wpm_state_ui ui;
static int64_t                 last_wpm_update_time = 0;
static int                     last_wpm             = -1;

#define WPM_UPDATE_INTERVAL_MS 250 // Throttle: max 4 updates per second

static void
set_wpm_state(int wpm)
{
	lv_obj_t *txt;
	int64_t   now;

	if (wpm == last_wpm)
	{
		return;
	}

	if (((now = k_uptime_get()) - last_wpm_update_time) < WPM_UPDATE_INTERVAL_MS)
	{
		return;
	}

	last_wpm_update_time = now;
	last_wpm             = wpm;

	txt = ui.txt;
	lv_label_set_text_fmt(txt, "%03d", wpm);
}

void
wpm_state_cb(struct wpm_state st)
{
	struct zmk_widget *widget;

	SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
	{
		if (widget->initialized)
		{
			set_wpm_state(st.wpm);
		}
	}
}

struct wpm_state
wpm_get_state(const zmk_event_t *eh)
{
	const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
	return (struct wpm_state){
	    .wpm = ev ? ev->state : 0,
	};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_state, struct wpm_state, wpm_state_cb, wpm_get_state)
ZMK_SUBSCRIPTION(widget_wpm_state, zmk_wpm_state_changed);

void
zmk_wpm_state_init(struct zmk_widget *widget, lv_obj_t *parent)
{
	lv_obj_t *txt;

	widget->obj = lv_obj_create(parent);

	txt = lv_label_create(parent);
	lv_label_set_text_fmt(txt, "%03d", 0);
	lv_obj_set_style_text_color(txt, lv_color_black(), 0);
	lv_obj_align(txt, LV_ALIGN_TOP_RIGHT, -2, 0);

	ui.txt = txt;

	widget_wpm_state_init();

	widget->initialized = true;
	sys_slist_append(&widgets, &widget->node);
}
