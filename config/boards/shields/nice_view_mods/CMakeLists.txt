if(CONFIG_ZMK_DISPLAY AND CONFIG_SHIELD_NICE_VIEW_MODS)
  zephyr_library_sources(custom_status_screen.c)
endif()
