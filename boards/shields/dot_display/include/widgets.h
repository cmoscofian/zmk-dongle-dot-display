#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget
{
	sys_snode_t node;
	lv_obj_t   *obj;
	bool        initialized;
};
