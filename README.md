# zmk-config-cornix

Personal ZMK firmware for the **Cornix** split keyboard, driven by a
**Seeed XIAO nRF52840 (Plus)** dongle.

## Topology

```
        ┌─────────────────────────┐
        │  XIAO nRF52840  (dongle) │   = BLE split CENTRAL + USB to host
        │  + Prospector ring disp. │     + 18 physical keys (3x6)
        │  + Raw HID (HID_1)       │
        └───────────┬─────────────┘
            BLE     │     BLE
        ┌───────────┴───┐   ┌───────────────┐
        │ cornix_ph_left │   │  cornix_right │   = peripherals (halves)
        │  (left half)   │   │  (right half) │     each with an EC11 encoder
        └────────────────┘   └───────────────┘     + RGB indicator LEDs
```

## Keymap position map (68 keys)

* `0..49`  — cornix split halves, reported by the two peripherals using the
  cornix board's own 50-key matrix transform.
* `50..67` — the 18 dongle keys (3 rows × 6 cols), appended *below* the cornix
  by giving the dongle's local matrix transform `row-offset = <4>` (the dongle
  scans local rows 0–2, which become global rows 4–6 → positions 50–67).

The keymap lives in [`config/cornix_dongle.keymap`](config/cornix_dongle.keymap)
(named after the `cornix_dongle` shield, so the peripheral builds keep using the
cornix board's built-in 50-key keymap).

## Dongle key matrix (XIAO pins)

`diode-direction = "col2row"` (columns = outputs, rows = inputs w/ pull-down):

| | pin |
|---|---|
| col0 | D2 / P0.28 |
| col1 | P0.15 |
| col2 | P0.19 |
| col3 | P1.01 |
| col4 | P0.09 (NFC1) |
| col5 | P0.10 (NFC2) |
| row0 | P1.07 |
| row1 | P1.05 |
| row2 | P1.03 |

P0.09 / P0.10 are the nRF52840 NFC pins, freed via `&uicr { nfct-pins-as-gpios; }`
and `CONFIG_NFCT_PINS_AS_GPIOS=y`.

## Encoders

Bound on every layer via a custom `&inc_dec_ms` (sensor-rotate → `&msc`):

* left encoder  → vertical mouse scroll (`SCRL_UP` / `SCRL_DOWN`)
* right encoder → horizontal mouse scroll (`SCRL_LEFT` / `SCRL_RIGHT`)

## Prospector (touch disabled — keycodes)

Touch hardware is left unused; control the ring display via keycodes (placed on
the Settings/`Debug_layer`):

| keycode | action |
|---|---|
| F20 | dark-theme toggle |
| F21 | AI-Usage screen toggle |
| F23 | brightness down |
| F24 | brightness up |

AI Usage / time / host layer control come from `zmk-rawhid-app`
(`RAWHID_APP_AI_USAGE` / `_TIME_SYNC` / `_LAYER_CONTROL`), fed by a host-side
Raw HID sender.

## Build targets (`build.yaml`)

| artifact | board / shields |
|---|---|
| `cornix_dongle` | `xiao_ble` + `cornix_dongle prospector_adapter raw_hid_adapter` |
| `cornix_left` | `cornix_ph_left` (left peripheral) |
| `cornix_right` | `cornix_right` (right peripheral) |
| `cornix_reset` | `cornix_right` + `settings_reset` |

## RGB indicator (not enabled yet)

RGB battery/connection indicators were requested but are **not enabled**: the
cornix module marks RGB as an unimplemented future item, and its
`cornix_indicator` shield references Kconfig symbols that no longer exist on the
current Zephyr 4.1 toolchain (`WS2812_STRIP` → `WS2812_STRIP_SPI`, and
`RGBLED_WIDGET_WS2812 / _LED_COUNT / _BRIGHTNESS / _LED_SHARING`, which exist only
in the `hitsmaxft` rgbled-widget fork). Enabling it would require patching the
cornix module itself (and validating against the real LED hardware), so it is
left as a follow-up.

## Building locally (from the workspace root)

```bash
just init config/zmk-config-cornix   # switch the active west manifest + west update
just build cornix                    # build all matching targets -> firmware/
```
