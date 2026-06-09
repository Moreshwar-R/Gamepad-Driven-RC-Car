## 🔍 Component Details

### 1. ESP32 Dev Board (30-pin, CH9102X)
- **Role:** Main microcontroller — runs Bluepad32, handles motor PWM, processes gamepad input over Bluetooth HID
- **Specs:** ESP32 dual-core @ 240 MHz, Bluetooth Classic + BLE, 30-pin, **CH9102X USB-to-UART** (faster, more reliable than CP2102)
- **Source:** [AIIOT Tech](https://aiiottech.in/product/30pin-ch9102x-esp-32-development-board/)
- **Price:** ₹430
- **Notes:** Driver auto-installs on Windows 11 / macOS Sequoia. For older OSes, install CH9102 driver from WCH website.

### 2. BTS7960 43 A Motor Driver (×2)
- **Role:** High-current H-bridge drivers — one per side of the bot, each driving two Johnson motors wired in parallel for 4WD tank-style propulsion
- **Specs:** 43 A peak, 5.5–27 V VM, dual half-bridge (RPWM/LPWM independently controlled), built-in current sense + thermal protection
- **Quantity:** 2
- **Source:** [AIIOT Tech](https://aiiottech.in/product/bts7960b-43a-h-bridge-motor-driver/)
- **Price:** ₹260 each
- **Why this driver:** Johnson motors stall around 4–5 A — the TB6612FNG (1.2 A) would burn out instantly. BTS7960 has massive headroom.

### 3. Johnson Geared DC Motors (×4)
- **Role:** Drive the four wheels — 2 per side, mechanically tied via shared wheel/track
- **Specs:** 12 V, 100 RPM, metal gearbox, 12 mm shaft
- **Quantity:** 4
- **Source:** [Robu.in](https://robu.in/product/motor-johnson-100-rpm/)
- **Price:** ₹449 each
- **Notes:** High torque, low RPM — ideal for tracked drive. Stall current ~4.5 A.

### 4. Tracked Wheels (11 × 4 cm) (×4)
- **Role:** Provide traction and absorb terrain unevenness
- **Specs:** 11 cm dia × 4 cm width, rubber tread, fits 12 mm Johnson shaft via coupling
- **Quantity:** 4
- **Source:** Richie Street, Chennai *(reference listing: [Factory Forward](https://www.factoryforward.com/product/11x4-cm-big-tracked-wheel-for-dc-geared-and-johnson-motor/))*
- **Price:** ₹135 each

### 5. Johnson Motor Couplings (12 mm) (×4)
- **Role:** Mechanical interface between Johnson shaft and wheel hub
- **Specs:** 12 mm bore, includes grub screws / nut + bolt
- **Quantity:** 4
- **Source:** Richie Street, Chennai
- **Price:** ₹60 each

### 6. Motor Clamps (×4)
- **Role:** Mounts each Johnson motor securely to the chassis
- **Specs:** Steel clamp with nut + bolt
- **Quantity:** 4
- **Source:** Richie Street, Chennai
- **Price:** ₹40 each

### 7. LM2596 Buck Converter
- **Role:** Steps 11.1 V LiPo down to 5 V for ESP32 logic
- **Specs:** 3 A max output, adjustable via onboard potentiometer
- **Source:** [AIIOT Tech](https://aiiottech.in/product/lm2596-dc-dc-buck-converter/)
- **Price:** ₹45
- **Notes:** ⚠️ **Set output to exactly 5.0 V with a multimeter BEFORE connecting the ESP32.** Out of the box it ships at random voltages.

### 8. 3S LiPo Battery (11.1 V, 2200 mAh)
- **Role:** Main power source — feeds motor drivers directly and the LM2596 for logic
- **Specs:** 11.1 V nominal (12.6 V full), 2200 mAh, 30 C discharge (≈66 A burst)
- **Source:** [FlyRobo](https://www.flyrobo.in/absd-lipo-battery-11.1v-2200mah-3s-30c)
- **Price:** ₹1600
- **Notes:** ⚠️ Use a **3S LiPo balance charger**. Never discharge below 9.0 V (3.0 V/cell). Store at 11.4 V (storage charge) if unused for more than a week.

### 9. Cosmic Byte Ares C3070W Gamepad
- **Role:** Wireless input — pairs to ESP32 over Bluetooth HID
- **Specs:** Dual analog sticks, D-pad, 4 face buttons, 2 triggers + 2 bumpers, 2.4 GHz / BT modes
- **Source:** Amazon.in
- **Price:** ₹ 1700
- **Notes:** Set the controller to its **Bluetooth pairing mode** (not 2.4 GHz dongle mode) for Bluepad32 to detect it.

### 10. Wooden Chassis
- **Role:** Physical platform — holds motors, battery, electronics deck
- **Specs:** Custom wooden 2-tier build
- **Source:** Local fabrication / Richie Street
- **Price:** ₹50

### 11. Perfboard
- **Role:** Soldered prototyping board for stable power distribution and pin headers
- **Source:** Richie Street, Chennai
- **Price:** ₹20

### 12. Wires, Fasteners & Misc
- Male-male and male-female jumper wires
- 14 AWG silicone wire for motor + battery leads
- M3 nuts, bolts, spacers
- Heat shrink tubing
- XT60 / T-Dean battery connector
- **Approx total:** ₹___

---