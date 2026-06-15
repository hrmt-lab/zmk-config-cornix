#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/battery.h>
#include <zmk/usb.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define WS2812_STRIP_NODE DT_CHOSEN(zmk_ws2812_widget)

#if !DT_NODE_EXISTS(WS2812_STRIP_NODE)
#error "Cornix charging indicator requires chosen node zmk,ws2812-widget"
#endif

static const struct device *const led_strip = DEVICE_DT_GET(WS2812_STRIP_NODE);
static const uint32_t num_pixels = DT_PROP(WS2812_STRIP_NODE, chain_length);

static struct led_rgb color_from_hex(uint32_t hex_color) {
    return (struct led_rgb){
        .r = (hex_color >> 16) & 0xff,
        .g = (hex_color >> 8) & 0xff,
        .b = hex_color & 0xff,
    };
}

static int set_leds(struct led_rgb color) {
    struct led_rgb pixels[num_pixels];

    for (uint32_t i = 0; i < num_pixels; i++) {
        pixels[i] = color;
    }

    return led_strip_update_rgb(led_strip, pixels, num_pixels);
}

static struct led_rgb scale_color(struct led_rgb color, uint16_t step) {
    return (struct led_rgb){
        .r = (color.r * step) / CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS,
        .g = (color.g * step) / CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS,
        .b = (color.b * step) / CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS,
    };
}

static bool should_breathe(void) {
    if (!zmk_usb_is_powered()) {
        return false;
    }

    uint8_t battery_level = zmk_battery_state_of_charge();
    return battery_level > 0 && battery_level < CONFIG_CORNIX_CHARGING_INDICATOR_FULL_LEVEL;
}

static void charging_indicator_thread(void *d0, void *d1, void *d2) {
    ARG_UNUSED(d0);
    ARG_UNUSED(d1);
    ARG_UNUSED(d2);

    if (!device_is_ready(led_strip)) {
        LOG_ERR("Cornix charging indicator LED strip not ready");
        return;
    }

    const struct led_rgb charging_color =
        color_from_hex(CONFIG_CORNIX_CHARGING_INDICATOR_COLOR);
    const struct led_rgb off = {0, 0, 0};

    while (true) {
        if (!should_breathe()) {
            set_leds(off);
            k_sleep(K_MSEC(CONFIG_CORNIX_CHARGING_INDICATOR_STEP_MS *
                           CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS));
            continue;
        }

        for (uint16_t step = 0; step <= CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS; step++) {
            if (!should_breathe()) {
                break;
            }
            set_leds(scale_color(charging_color, step));
            k_sleep(K_MSEC(CONFIG_CORNIX_CHARGING_INDICATOR_STEP_MS));
        }

        for (uint16_t step = CONFIG_CORNIX_CHARGING_INDICATOR_BREATH_STEPS; step > 0; step--) {
            if (!should_breathe()) {
                break;
            }
            set_leds(scale_color(charging_color, step - 1));
            k_sleep(K_MSEC(CONFIG_CORNIX_CHARGING_INDICATOR_STEP_MS));
        }

        set_leds(off);
        k_sleep(K_MSEC(CONFIG_CORNIX_CHARGING_INDICATOR_OFF_HOLD_MS));
    }
}

K_THREAD_DEFINE(charging_indicator_tid, 1024, charging_indicator_thread, NULL, NULL, NULL,
                K_LOWEST_APPLICATION_THREAD_PRIO, 0, 500);
