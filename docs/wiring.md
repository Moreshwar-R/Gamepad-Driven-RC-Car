# 🔌 Wiring Guide

Complete pin-by-pin wiring reference for the **ESP32 Bluetooth RC Car**.
This document maps every connection from the ESP32 to the BTS7960 motor drivers, motors, and power rails.

---

## 🧠 ESP32 → BTS7960 (Motor Drivers ×2)

| ESP32 GPIO | BTS7960 Silkscreen | Signal Type | Function |
|-----------:|:-------------------|:------------|:---------|
| GPIO 12 | **L_RPWM** | PWM out | Left driver — forward PWM |
| GPIO 14 | **L_LPWM** | PWM out | Left driver — reverse PWM |
| GPIO 13 | **R_RPWM** | PWM out | Right driver — forward PWM |
| GPIO 26 | **R_LPWM** | PWM out | Right driver — reverse PWM |
| GPIO 33 | **L_EN + R_EN (both drivers)** | Digital out | Master enable — HIGH = bridges on, LOW = full coast |
| 5 V | **VCC (both drivers)** | Power | Logic-side power for BTS7960 control inputs |
| GND | **GND (both drivers)** | — | Common ground |

> ⚙️ All four PWM pins use the ESP32's **LEDC hardware PWM peripheral** at **20 kHz, 8-bit** — above the audible band and well inside the BTS7960's 25 kHz max switching spec.

> 🔗 **Tying the enables**: jumper `L_EN` and `R_EN` together on each driver, then run a single wire from that pair on each driver back to **GPIO 33**. Four BTS7960 enable pins total → one ESP32 GPIO.

---

## ⚙️ BTS7960 → Motors

Each BTS7960 has two motor output terminals labelled **M+** and **M−** (sometimes silkscreened **B+ / B−** for "motor B+/B-"; check yours).

| Driver | Motor terminal | Wired to |
|--------|---------------|----------|
| Left BTS7960  | M+, M− | Both left-side Johnson motors, in parallel |
| Right BTS7960 | M+, M− | Both right-side Johnson motors, in parallel |

> 💡 **If a side spins the wrong direction**, swap that driver's M+ and M− leads. No code change needed.

---

## 🔋 Power Distribution

```
                    ┌─────────────────┐
                    │  3S LiPo 11.1V  │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
       ┌──────▼──────┐ ┌─────▼─────┐ ┌──────▼──────┐
       │  LM2596     │ │  Left     │ │  Right      │
       │  11.1V → 5V │ │  BTS7960  │ │  BTS7960    │
       │             │ │   VM (B+) │ │   VM (B+)   │
       └──────┬──────┘ └───────────┘ └─────────────┘
              │
        ┌─────┴─────────────────────┐
        │                           │
   ┌────▼────┐         ┌────────────▼────────────┐
   │ ESP32   │         │  BTS7960 ×2 VCC (5 V)   │
   │  VIN    │         │      (logic power)      │
   └─────────┘         └─────────────────────────┘
```

### ⚠️ Critical wiring rules

- **All grounds tied together** — ESP32 GND, both BTS7960 GNDs, LM2596 GND, and battery negative share one common ground rail.
- **Set the LM2596 to 5.0 V** with a multimeter **before** connecting the ESP32. Out of the box, the buck converter ships at random voltages.
- **VM (B+) ≠ VCC.** VM is raw battery voltage feeding the H-bridge; VCC is the 5 V logic supply for the control side. Don't swap them — putting 11.1 V on VCC will fry the BTS7960's logic.
- **Each BTS7960 draws its own VM line directly from the battery** — these things pull 4+ A per motor under stall, daisy-chaining them through a single thin wire will brown out the second driver.
- **EN pins MUST be high to move motors.** If GPIO 33 is floating or LOW, both bridges are in coast mode and nothing happens. The firmware deliberately holds EN low until a controller pairs.

---

## 🧩 Full System Wiring Diagram

```
┌──────────────┐ BT HID  ┌──────────────────┐
│ Cosmic Byte  │ ──────► │   ESP32 DevKit   │
│   Gamepad    │         │       V1         │
└──────────────┘         └────────┬─────────┘
                                  │
            ┌─────────────────────┼─────────────────────┐
            │                     │                     │
            │ PWM ×2              │ PWM ×2              │ enable
            │ (GPIO 12, 14)       │ (GPIO 13, 26)       │ (GPIO 33)
            │                     │                     │
   ┌────────▼────────┐   ┌────────▼────────┐            │
   │   BTS7960 LEFT  │   │  BTS7960 RIGHT  │ ◄──────────┤
   │  L_RPWM, L_LPWM │   │  R_RPWM, R_LPWM │            │
   │  L_EN+R_EN tied │   │  L_EN+R_EN tied │ ◄──────────┘
   └────────┬────────┘   └────────┬────────┘
            │                     │
       M+/M−│                     │M+/M−
            │                     │
   ┌────────▼────────┐   ┌────────▼────────┐
   │ Johnson ×2      │   │ Johnson ×2      │
   │  (left side)    │   │  (right side)   │
   └─────────────────┘   └─────────────────┘
```

---

## 📝 Practical Wiring Notes

- **Decoupling capacitor (100 µF, electrolytic)** across each BTS7960's VM ↔ GND is strongly recommended — absorbs back-EMF spikes from the motors that can otherwise reset the ESP32 over the shared ground.
- **Keep motor wires short** to reduce EMI on the ESP32's Bluetooth antenna. Twist each motor's two leads together if you can.
- **14 AWG silicone wire** for the battery → driver power path. Thin wires create voltage drop under load — especially critical with BTS7960s that can pull 40+ A peak.
- **Solder all power connections.** Loose breadboard joints sag under stall current and can desolder themselves from the heat.
- **EN tie-point**: the cleanest place to tie L_EN + R_EN together is right on each BTS7960's pin header — a single jumper wire across the two adjacent pins, then a single signal wire from each driver back to GPIO 33.

---

## 🧪 Bring-Up Checklist

Before powering the bot for the first time:

- [ ] All grounds tied together (ESP32, both BTS7960s, LM2596, battery negative)
- [ ] LM2596 output measured at 5.0 V (with multimeter, **before** ESP32 connected)
- [ ] L_EN + R_EN jumpered on each BTS7960, and both driver pairs wired to GPIO 33
- [ ] All four PWM lines (GPIO 12, 14, 13, 26) confirmed at the correct RPWM/LPWM pin per side
- [ ] VCC (5 V) and VM (battery) **not** swapped — measure with multimeter before applying battery
- [ ] Motor outputs match expected sides (test by flashing `02_rc_drive` and pushing forward)
- [ ] Battery connector polarity verified — red to VM+, black to VM−
- [ ] First power-on done **without motors attached** to verify ESP32 boots normally and EN stays LOW until pairing
