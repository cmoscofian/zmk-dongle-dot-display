#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/peripheral_conn_state_changed.h>

#include <fonts.h>
#include <widgets.h>
#include <widgets/peripheral_battery_state.h>

LOG_MODULE_REGISTER(peripheral_battery_state, LOG_LEVEL_INF);

#ifndef ZMK_SPLIT_BLE_PERIPHERAL_COUNT
#define ZMK_SPLIT_BLE_PERIPHERAL_COUNT 2
#endif

static sys_slist_t                            widgets                                               = SYS_SLIST_STATIC_INIT(&widgets);
static uint8_t                                peripheral_batt_state[ZMK_SPLIT_BLE_PERIPHERAL_COUNT] = {0};
static bool                                   peripheral_conn_state[ZMK_SPLIT_BLE_PERIPHERAL_COUNT] = {0};
static struct zmk_peripheral_battery_state_ui ui[ZMK_SPLIT_BLE_PERIPHERAL_COUNT]                    = {0};

static void
update_peripheral_battery_state(uint8_t source)
{
	lv_obj_t *txt;
	uint8_t   conn, batt;

	if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT)
	{
		return;
	}

	conn = peripheral_conn_state[source];
	batt = peripheral_batt_state[source];
	txt  = ui[source].txt;

	if (!conn || !batt)
	{
		lv_label_set_text(txt, LV_SYMBOL_CLOSE);
		lv_obj_set_style_text_font(txt, &lv_font_montserrat_28, 0);
	}
	else if (batt > 99)
	{
		lv_label_set_text(txt, LV_SYMBOL_CHARGE);
		lv_obj_set_style_text_font(txt, &lv_font_montserrat_28, 0);
	}
	else if (batt < 5)
	{
		lv_label_set_text(txt, LV_SYMBOL_WARNING);
		lv_obj_set_style_text_font(txt, &lv_font_montserrat_28, 0);
	}
	else
	{
		lv_label_set_text_fmt(txt, "%02d", batt);
		lv_obj_set_style_text_font(txt, &Symbols_Medium_28, 0);
	}
}

void
peripheral_battery_state_cb(struct peripheral_battery_state st)
{
	struct zmk_widget *widget;
	uint8_t            source;

	source = st.source;
	SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
	{
		if (widget->initialized)
		{
			if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT)
			{
				return;
			}
			peripheral_batt_state[source] = st.status;
			update_peripheral_battery_state(source);
		}
	}
}

struct peripheral_battery_state
peripheral_battery_get_state(const zmk_event_t *eh)
{
	if (!eh)
	{
		return (struct peripheral_battery_state){
		    .source = 0,
		    .status = 0,
		};
	}

	const struct zmk_peripheral_battery_state_changed *ev = as_zmk_peripheral_battery_state_changed(eh);
	if (!ev)
	{
		return (struct peripheral_battery_state){
		    .source = 0,
		    .status = 0,
		};
	}

	return (struct peripheral_battery_state){
	    .source = ev->source,
	    .status = ev->state_of_charge,
	};
}

void
peripheral_conn_state_cb(struct peripheral_conn_state st)
{
	struct zmk_widget *widget;
	uint8_t            source;

	source = st.source;
	SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
	{
		if (widget->initialized)
		{
			if (source >= ZMK_SPLIT_BLE_PERIPHERAL_COUNT)
			{
				return;
			}
			peripheral_conn_state[source] = st.conn;
			update_peripheral_battery_state(source);
		}
	}
}

struct peripheral_conn_state
peripheral_conn_get_state(const zmk_event_t *eh)
{
	if (!eh)
	{
		return (struct peripheral_conn_state){
		    .source = 0,
		    .conn   = false,
		};
	}

	const struct zmk_peripheral_conn_state_changed *ev = as_zmk_peripheral_conn_state_changed(eh);
	if (!ev)
	{
		return (struct peripheral_conn_state){
		    .source = 0,
		    .conn   = false,
		};
	}

	return (struct peripheral_conn_state){
	    .source = ev->source,
	    .conn   = ev->connected,
	};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_battery_state, struct peripheral_battery_state, peripheral_battery_state_cb, peripheral_battery_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_battery_state, zmk_peripheral_battery_state_changed);
ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_conn_state, struct peripheral_conn_state, peripheral_conn_state_cb, peripheral_conn_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_conn_state, zmk_peripheral_conn_state_changed);

void
zmk_peripheral_battery_state_init(struct zmk_widget *widget, lv_obj_t *parent)
{
	int       i;
	lv_obj_t *box;
	lv_obj_t *txt;
	lv_obj_t *sym;

	widget->obj = lv_obj_create(parent);

	for (i = 0; i < ZMK_SPLIT_BLE_PERIPHERAL_COUNT; ++i)
	{
		box = lv_obj_create(parent);
		lv_obj_set_size(box, 60, 50);
		lv_obj_set_style_border_color(box, lv_color_black(), LV_PART_MAIN);
		lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
		lv_obj_set_style_radius(box, 5, LV_PART_MAIN);
		lv_obj_align(box, LV_ALIGN_BOTTOM_LEFT, i * 62 + 2, 0);

		txt = lv_label_create(box);
		lv_label_set_text(txt, LV_SYMBOL_REFRESH);
		lv_obj_set_style_text_color(txt, lv_color_black(), 0);
		lv_obj_set_style_text_font(txt, &lv_font_montserrat_28, 0);
		lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);

		sym = lv_label_create(box);
		lv_label_set_text(sym, "%");
		lv_obj_set_style_text_color(sym, lv_color_black(), 0);
		lv_obj_align(sym, LV_ALIGN_BOTTOM_RIGHT, -2, -2);

		ui[i].box = box;
		ui[i].sym = sym;
		ui[i].txt = txt;
	}

	widget_peripheral_battery_state_init();
	widget_peripheral_conn_state_init();

	widget->initialized = true;
	sys_slist_append(&widgets, &widget->node);
}
