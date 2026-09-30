#include <lvgl.h>
#include <zephyr/logging/log.h>

#include <widgets.h>
#include <widgets/layer_state.h>
#include <widgets/peripheral_battery_state.h>
#include <widgets/wpm_state.h>

LOG_MODULE_REGISTER(dot_display, LOG_LEVEL_INF);

static struct zmk_widget battery;
static struct zmk_widget layer;
static struct zmk_widget wpm;

lv_obj_t *
zmk_display_status_screen()
{
	lv_obj_t *screen;

	screen = lv_obj_create(NULL);

	lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
	lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

	zmk_layer_state_init(&battery, screen);
	zmk_peripheral_battery_state_init(&layer, screen);
	zmk_wpm_state_init(&wpm, screen);

	return screen;
}
