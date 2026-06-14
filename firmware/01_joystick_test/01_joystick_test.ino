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
