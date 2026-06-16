/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include <lvgl.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/keys.h>
#include <zmk/split/bluetooth/peripheral.h>
#include <zmk/usb.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static lv_obj_t *status_label;
static lv_obj_t *mods_label;

static zmk_mod_flags_t active_modifiers;
static uint8_t battery_level;
static bool peripheral_connected;

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

struct peripheral_status_state {
    bool connected;
};

struct modifiers_status_state {
    zmk_mod_flags_t modifiers;
    bool state;
};

static void set_label(lv_obj_t *label, const char *text) {
    if (label != NULL) {
        lv_label_set_text(label, text);
    }
}

static void draw_status(void) {
    char text[20];
    snprintf(text, sizeof(text), "%s %d%%", peripheral_connected ? "LINK" : "NO LINK",
             battery_level);
    set_label(status_label, text);
}

static bool has_any_mod(zmk_mod_flags_t modifiers, zmk_mod_flags_t left, zmk_mod_flags_t right) {
    return (modifiers & (left | right)) != 0;
}

static void draw_modifiers(void) {
    char text[16];
    snprintf(text, sizeof(text), "%c %c %c %c",
             has_any_mod(active_modifiers, MOD_LCTL, MOD_RCTL) ? 'C' : '-',
             has_any_mod(active_modifiers, MOD_LALT, MOD_RALT) ? 'A' : '-',
             has_any_mod(active_modifiers, MOD_LGUI, MOD_RGUI) ? 'G' : '-',
             has_any_mod(active_modifiers, MOD_LSFT, MOD_RSFT) ? 'S' : '-');
    set_label(mods_label, text);
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *_eh) {
    return (struct battery_status_state){
        .level = zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif
    };
}

static void battery_status_update_cb(struct battery_status_state state) {
    battery_level = state.level;
    draw_status();
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif

static struct peripheral_status_state peripheral_status_get_state(const zmk_event_t *_eh) {
    return (struct peripheral_status_state){.connected = zmk_split_bt_peripheral_is_connected()};
}

static void peripheral_status_update_cb(struct peripheral_status_state state) {
    peripheral_connected = state.connected;
    draw_status();
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_status, struct peripheral_status_state,
                            peripheral_status_update_cb, peripheral_status_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_status, zmk_split_peripheral_status_changed);

static struct modifiers_status_state modifiers_status_get_state(const zmk_event_t *eh) {
    const struct zmk_modifiers_state_changed *ev = as_zmk_modifiers_state_changed(eh);

    return (struct modifiers_status_state){
        .modifiers = ev == NULL ? 0 : ev->modifiers,
        .state = ev != NULL && ev->state,
    };
}

static void modifiers_status_update_cb(struct modifiers_status_state state) {
    if (state.state) {
        active_modifiers |= state.modifiers;
    } else {
        active_modifiers &= ~state.modifiers;
    }

    draw_modifiers();
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_modifiers_status, struct modifiers_status_state,
                            modifiers_status_update_cb, modifiers_status_get_state)
ZMK_SUBSCRIPTION(widget_modifiers_status, zmk_modifiers_state_changed);

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    status_label = lv_label_create(screen);
    lv_obj_set_width(status_label, 156);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 2);

    lv_obj_t *title_label = lv_label_create(screen);
    lv_obj_set_width(title_label, 156);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(title_label, "MODS");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 24);

    mods_label = lv_label_create(screen);
    lv_obj_set_width(mods_label, 156);
    lv_obj_set_style_text_font(mods_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(mods_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(mods_label, LV_ALIGN_BOTTOM_MID, 0, -4);

    widget_battery_status_init();
    widget_peripheral_status_init();
    widget_modifiers_status_init();

    draw_status();
    draw_modifiers();

    return screen;
}
