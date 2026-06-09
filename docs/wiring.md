# 🔌 Wiring Guide

Complete pin-by-pin wiring reference for the **ESP32 Bluetooth RC Car**.
This document maps every connection from the ESP32 to the motor driver, sensors, and power rails.

---

## 🧠 ESP32 → TB6612FNG (Motor Driver)

| ESP32 GPIO | TB6612FNG Pin | Signal Type | Function |
|-----------:|:--------------|:------------|:---------|
| GPIO 12 | PWMA | PWM | Motor A speed control |
| GPIO 14 | AIN1 | Digital out | Motor A direction bit 1 |
| GPIO 27 | AIN2 | Digital out | Motor A direction bit 2 |
| GPIO 13 | PWMB | PWM | Motor B speed control |
| GPIO 26 | BIN1 | Digital out | Motor B direction bit 1 |
| GPIO 25 | BIN2 | Digital out | Motor B direction bit 2 |
| GPIO 33 | STBY | Digital out | Driver enable (HIGH = on, LOW = off) |
| GND | GND | — | Common ground |

> ⚙️ The PWMA / PWMB pins use the ESP32's **LEDC hardware PWM peripheral**, configured at 1 kHz with 8-bit resolution.

---

## ⚙️ Motor Driver → Motors

| Driver Output | Motor |
|---------------|-------|
| AO1, AO2 | Left motor |
| BO1, BO2 | Right motor |

> 💡 **If a motor spins the wrong direction**, just swap its two output wires. No code change needed.

---

## 🔋 Power Distribution

```
                    ┌─────────────────┐
                    │  3S LiPo 11.1V  │
                    └────────┬────────┘
                             │
                ┌────────────┴────────────┐
                │                         │
        ┌───────▼───────┐         ┌───────▼─────────┐
        │  LM2596 Buck  │         │   TB6612FNG VM  │
        │   11.1V → 5V  │         │   (motor power) │
        └───────┬───────┘         └─────────────────┘
                │
        ┌───────┴────────┐
        │                │
   ┌────▼────┐    ┌──────▼──────────┐
   │ ESP32   │    │  TB6612FNG VCC  │
   │  VIN    │    │  (logic power)  │
   └─────────┘    └─────────────────┘
```

### ⚠️ Critical wiring rules

- **All grounds must be tied together** — ESP32 GND, TB6612FNG GND, LM2596 GND, and battery negative all share one common ground rail.
- **Set the LM2596 to 5.0 V** with a multimeter **before** connecting the ESP32. Out of the box, the buck converter ships at random voltages.
- **VM = battery voltage** (motor power) and **VCC = 5 V** (logic power) are *separate* — don't connect both to the same rail.

---

## 🧩 Full System Wiring Diagram

```
┌──────────────┐ BT HID  ┌──────────────┐
│ Cosmic Byte  │ ──────► │   ESP32      │
│   Gamepad    │         │   DevKit V1  │
└──────────────┘         └──────┬───────┘
                                │
                  ┌─────────────┼─────────────┐
                  │             │             │
            ┌─────▼─────┐ ┌─────▼─────┐ ┌─────▼─────┐
            │ PWMA/AIN  │ │ PWMB/BIN  │ │   STBY    │
            │ GPIO 12,  │ │ GPIO 13,  │ │  GPIO 33  │
            │  14, 27   │ │  26, 25   │ │           │
            └─────┬─────┘ └─────┬─────┘ └─────┬─────┘
                  │             │             │
                  └─────────────┼─────────────┘
                                │
                       ┌────────▼─────────┐
                       │    TB6612FNG     │
                       │   Dual H-Bridge  │
                       └────┬─────────┬───┘
                            │         │
                       ┌────▼───┐ ┌───▼────┐
                       │ Motor A│ │ Motor B│
                       │ (Left) │ │(Right) │
                       └────────┘ └────────┘
```

---

## 📝 Practical Wiring Notes

- **Decoupling capacitor (100 µF)** across the motor power rail is strongly recommended — absorbs back-EMF spikes from the motors that can otherwise reset the ESP32.
- **Keep motor wires short** to reduce EMI interference on the ESP32's Bluetooth antenna. Twist the motor wire pairs if possible.
- **Use 14 AWG silicone wire** for the battery → driver power path. Thin wires create voltage drop under load.
- **Solder all power connections** if possible — loose breadboard connections sag under the current draw of stalled motors.

---

## 🧪 Bring-Up Checklist

Before powering the bot for the first time:

- [ ] All grounds tied together
- [ ] LM2596 output measured at 5.0 V (with multimeter, **before** ESP32 connected)
- [ ] STBY wire confirmed connected (motors won't move without it HIGH)
- [ ] Motor outputs match expected sides (test by flashing `02_rc_drive` and pushing forward)
- [ ] Battery connector polarity verified — red to VM+, black to VM−
- [ ] First power-on done **without motors attached** to verify ESP32 boots normally