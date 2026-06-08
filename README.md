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

## RGB indicator

Each half has its own RGB LEDs that show **that half's battery and connection
status**. They are normally off and only light up briefly when something
happens, so they don't drain the battery.

**Battery (shown when you turn the half on):**

| Color | Meaning |
|---|---|
| 🟢 Green | Battery high (80%+) |
| 🟡 Yellow | Battery medium (20–80%) |
| 🔴 Red | Battery low / critical (under 20%) |

**Connection (shown when the half connects to or drops from the dongle):**

| Color | Meaning |
|---|---|
| 🔵 Blue | Connected to the dongle |
| 🔴 Red | Not connected |

Notes:

- The LEDs do **not** show the active layer or the Bluetooth profile. Those are
  only known by the dongle, not by the halves where the LEDs are, so they can't
  be displayed there.
- Only the two halves (`cornix_left` / `cornix_right`) have LEDs — the dongle has
  none. After this change you only need to reflash the halves; the dongle is
  unchanged.

## Building locally (from the workspace root)

```bash
just init config/zmk-config-cornix   # switch the active west manifest + west update
just build cornix                    # build all matching targets -> firmware/
```
