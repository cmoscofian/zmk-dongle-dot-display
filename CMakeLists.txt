zephyr_library()
zephyr_include_directories(include)
zephyr_library_sources(src/events/peripheral_conn_state_changed.c)
zephyr_library_sources(src/split/bluetooth/peripheral_conn_state_changed.c)

# vim: ft=cmake
