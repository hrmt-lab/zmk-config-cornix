# zmk-config-cornix

[English](#english)

Cornix split keyboard を **Seeed XIAO nRF52840 (Plus)** dongle で使うための
個人用 ZMK firmware 設定です。

## 構成

```
        ┌─────────────────────────┐
        │  XIAO nRF52840  (dongle) │   = BLE split central + host へ USB 接続
        │  + Prospector ring disp. │     + 18 physical keys (3x6)
        │  + Raw HID (HID_1)       │
        └───────────┬─────────────┘
            BLE     │     BLE
        ┌───────────┴───┐   ┌───────────────┐
        │ cornix_ph_left │   │  cornix_right │   = peripheral として動作する左右半身
        │ (left half)    │   │  (left half)  │     各半身に EC11 encoder
        └────────────────┘   └───────────────┘     + RGB indicator LED
```

## キーマップ位置

- `0..49`: Cornix 左右半身の 50 key matrix
- `50..67`: dongle 側の 18 keys (3 rows x 6 cols)

キーマップは [`config/cornix_dongle.keymap`](config/cornix_dongle.keymap) にあります。

## Dongle key matrix

`diode-direction = "col2row"` です。

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

P0.09 / P0.10 は nRF52840 の NFC pin なので、`&uicr { nfct-pins-as-gpios; }`
と `CONFIG_NFCT_PINS_AS_GPIOS=y` で GPIO として使っています。

## Encoder

各 layer で `&inc_dec_ms` を使い、encoder を mouse scroll に割り当てています。

- 左 encoder: 縦 scroll (`SCRL_UP` / `SCRL_DOWN`)
- 右 encoder: 横 scroll (`SCRL_LEFT` / `SCRL_RIGHT`)

## Prospector

Touch hardware は使わず、Settings / `Debug_layer` 上の keycode で ring display を操作します。

| keycode | action |
|---|---|
| F20 | dark theme toggle |
| F21 | AI Usage screen toggle |
| F23 | brightness down |
| F24 | brightness up |

AI Usage / time / host layer control は `zmk-rawhid-app`
(`RAWHID_APP_AI_USAGE` / `_TIME_SYNC` / `_LAYER_CONTROL`) 経由です。

## Build targets

| artifact | board / shields |
|---|---|
| `cornix_dongle` | `xiao_ble` + `cornix_dongle prospector_adapter raw_hid_adapter` |
| `cornix_left` | `cornix_ph_left` (left peripheral) |
| `cornix_right` | `cornix_right` (right peripheral) |
| `cornix_reset` | `cornix_right` + `settings_reset` |

## RGB indicator

左右半身には RGB LED があり、それぞれの半身自身の **battery / connection /
USB 給電中の充電中相当表示** を行います。通常は消灯し、起動時や接続状態の変化時だけ
短く点灯します。ただし USB 給電中かつ battery 推定値が 100% 未満の場合は
breathing 表示を続けます。

**Battery (半身の電源 ON 時に表示):**

| Color | Meaning |
|---|---|
| Green | Battery high (80%+) |
| Yellow | Battery medium (20-80%) |
| Red | Battery low / critical (under 20%) |

**Connection (dongle との接続/切断時に表示):**

| Color | Meaning |
|---|---|
| Blue | Connected to the dongle |
| Red | Not connected |

**USB 給電中の充電中相当表示:**

| LED pattern | Meaning |
|---|---|
| Green breathing | USB 給電あり、かつ battery 推定値が 100% 未満 |
| Off | USB 未接続、または battery 推定値が 100% |

Breathing は約 0.84s で明るくなり、約 0.84s で暗くなり、その後約 0.84s
消灯してから繰り返します。

Notes:

- これは charger IC の状態信号ではなく、USB 給電を使った「充電中相当」表示です。
  実際に充電電流が流れているか、満充電で充電停止しているかまでは判別できません。
- 半身の物理電源 switch が OFF でも USB を接続すると LED が点灯することがあります。
  USB が LED 側に給電できるためで、firmware から shutdown 状態の LED を完全には
  制御できません。
- LED は active layer や Bluetooth profile は表示しません。それらは dongle 側の状態で、
  LED がある左右半身には転送されないためです。
- LED があるのは左右半身 (`cornix_left` / `cornix_right`) だけです。dongle にはありません。
  LED 表示変更だけなら、焼き直しは左右半身のみで OK です。

## ローカル build

Workspace root から実行します。

```bash
just init config/zmk-config-cornix
just build cornix
```

## English

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

Each half has its own RGB LEDs that show **that half's battery, connection, and
USB-powered charging-like status**. They are normally off and only light up
briefly when something happens, except while USB power is present and the half's
battery estimate is below 100%.

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

**USB-powered charging-like status:**

| LED pattern | Meaning |
|---|---|
| 🟢 Green breathing | USB power is present and battery estimate is under 100% |
| Off | USB is not present, or battery estimate is 100% |

The breathing pattern fades in for about 0.84s, fades out for about 0.84s, then
stays off for about 0.84s before repeating.

Notes:

- This is a USB-powered indicator, not a charger IC status signal. It means
  "charging-like" in normal use, but it cannot distinguish active charging from
  USB power with a full or disconnected battery.
- If USB is connected while the half's physical power switch is off, the LEDs may
  still light because USB can power the LED path even though the firmware cannot
  reliably control shutdown-state behavior.
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
