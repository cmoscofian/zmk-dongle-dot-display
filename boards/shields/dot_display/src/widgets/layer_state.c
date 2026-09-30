#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

#include <fonts.h>
#include <widgets.h>
#include <widgets/layer_state.h>

LOG_MODULE_REGISTER(layer_state, LOG_LEVEL_INF);

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static struct zmk_layer_state_ui ui;

static void
set_layer_state(const char *label)
{
	lv_obj_t *txt;

	txt = ui.txt;
	lv_label_set_text(txt, label);
}

void
layer_state_cb(struct layer_state st)
{
	struct zmk_widget *widget;

	SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
	{
		if (widget->initialized)
		{
			set_layer_state(st.label);
		}
	}
}

struct layer_state
layer_get_state(const zmk_event_t *eh)
{

	uint8_t index;

	index = zmk_keymap_highest_layer_active();

	return (struct layer_state){
	    .layer = index,
	    .label = zmk_keymap_layer_name(index),
	};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_state, struct layer_state, layer_state_cb, layer_get_state)
ZMK_SUBSCRIPTION(widget_layer_state, zmk_layer_state_changed);

void
zmk_layer_state_init(struct zmk_widget *widget, lv_obj_t *parent)
{
	lv_obj_t *txt;

	widget->obj = lv_obj_create(parent);

	txt = lv_label_create(parent);
	lv_label_set_text(txt, zmk_keymap_layer_name(0));
	lv_obj_set_style_text_color(txt, lv_color_black(), 0);
	lv_obj_align(txt, LV_ALIGN_TOP_LEFT, 2, 0);

	ui.txt = txt;

	widget_layer_state_init();

	widget->initialized = true;
	sys_slist_append(&widgets, &widget->node);
}
