/*
 * ============================================================================
 *  01_joystick_test.ino — Gamepad ↔ ESP32 Link Verification
 * ============================================================================
 *  Project : ESP32 Bluetooth RC Car
 *  Stage   : 1 of 2  (wireless link bring-up — NO motors yet)
 *  Author  : Moreshwar R
 *  Board   : ESP32 Dev Module (30-pin, CH9102X)
 *  Library : Bluepad32 (install via Arduino Library Manager)
 *
 *  Purpose
 *  -------
 *  Confirms the Cosmic Byte Ares C3070W (or any Bluetooth HID gamepad)
 *  pairs cleanly with the ESP32 and streams valid stick + button data
 *  over serial. Flash this BEFORE the drive sketch — if this prints
 *  clean values, the wireless link is solid.
 *
 *  Serial Output
 *  -------------
 *  - Firmware version + ESP32 Bluetooth (BD) address on boot
 *  - Live left-stick-Y and right-stick-X values @ 100 Hz
 *  - Button A (Cross) press events
 *
 *  Baud rate: 115200
 * ============================================================================
 */

#include <Bluepad32.h>

ControllerPtr myController;

// ---------------------------------------------------------------------------
//  Controller pair / unpair callbacks
// ---------------------------------------------------------------------------
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    Serial.print("CALLBACK: Controller is connected, index=");
    Serial.println(ctl->index());
    myController = ctl;
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("CALLBACK: Controller disconnected");
    myController = nullptr;
  }
}

// ---------------------------------------------------------------------------
//  Setup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Address: %2X:%2X:%2X:%2X:%2X:%2X\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Register pair / unpair callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // Disable virtual devices (we only want real gamepads)
  BP32.enableVirtualDevice(false);
}

// ---------------------------------------------------------------------------
//  Main loop — poll controller every 10 ms
// ---------------------------------------------------------------------------
void loop() {
  // Must be called every loop to refresh controller state
  BP32.update();

  if (myController && myController->isConnected()) {
    if (myController->isGamepad()) {
      processGamepad(myController);
    }
  }

  delay(10);  // 100 Hz polling — keeps serial from flooding
}

// ---------------------------------------------------------------------------
//  Read sticks + buttons and stream to serial
// ---------------------------------------------------------------------------
void processGamepad(ControllerPtr ctl) {
  // Raw stick values — Bluepad32 range: [-511, 512]
  int leftStickY  = ctl->axisY();    // throttle axis (forward / backward)
  int rightStickX = ctl->axisRX();   // steering axis (left / right)

  // Deadzone — kill idle drift when sticks are untouched
  if (abs(leftStickY)  < 15) leftStickY  = 0;
  if (abs(rightStickX) < 15) rightStickX = 0;

  Serial.print("Left Stick Y: ");  Serial.print(leftStickY);
  Serial.print("\tRight Stick X: "); Serial.println(rightStickX);

  // Button A (Cross on PS layout, A on Xbox layout)
  if (ctl->a()) {
    Serial.println("Button A (Cross) pressed!");
  }
}
