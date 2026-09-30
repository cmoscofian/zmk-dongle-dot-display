if(CONFIG_SHIELD_DOT_DISPLAY)
	zephyr_library()
	zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
	zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
	zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
	zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
	zephyr_library_include_directories(include)

	file(GLOB custom_fonts src/fonts/*.c)

	zephyr_library_sources(${custom_fonts})
	zephyr_library_sources(src/main.c)
	zephyr_library_sources(src/widgets/layer_state.c)
	zephyr_library_sources(src/widgets/peripheral_battery_state.c)
	zephyr_library_sources(src/widgets/wpm_state.c)
endif()

# vim: ft=cmake
