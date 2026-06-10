/*
 * ============================================================================
 *  02_rc_drive.ino — Full Motion Control (Arcade-Drive Mixing)
 * ============================================================================
 *  Project : ESP32 Bluetooth RC Car
 *  Stage   : 2 of 2  (full drive firmware — motors live)
 *  Author  : Moreshwar R
 *  Board   : ESP32 Dev Module (30-pin, CH9102X)
 *  Library : Bluepad32 (install via Arduino Library Manager)
 *
 *  Purpose
 *  -------
 *  Converts two analog joystick axes into smooth, steerable wheel motion
 *  using arcade-drive mixing. Drives a TB6612FNG dual H-bridge with
 *  hardware PWM via the ESP32 LEDC peripheral.
 *
 *  Control Logic
 *  -------------
 *      throttle  = -axisY        (invert so stick-up = forward)
 *      steering  =  axisRX
 *      leftWheel  = throttle + steering
 *      rightWheel = throttle - steering
 *
 *  Range remap : Bluepad32 raw [-511, 512] → PWM duty [-255, 255]
 *
 *  Safety Interlocks
 *  -----------------
 *  - 20-count deadzone on both axes (idle stick drift rejection)
 *  - STBY pin pulled LOW on controller disconnect (H-bridge gated off)
 *  - Per-motor constrain() to clip overflow / runaway
 *
 *  Baud rate: 115200
 * ============================================================================
 */

#include <Bluepad32.h>

// ---------------------------------------------------------------------------
//  Pin Map — ESP32 → TB6612FNG
// ---------------------------------------------------------------------------
#define PWMA  12   // Motor A speed (PWM)
#define AIN1  14   // Motor A direction 1
#define AIN2  27   // Motor A direction 2
#define PWMB  13   // Motor B speed (PWM)
#define BIN1  26   // Motor B direction 1
#define BIN2  25   // Motor B direction 2
#define STBY  33   // Standby — HIGH = enable, LOW = motors off

// ---------------------------------------------------------------------------
//  PWM Configuration
// ---------------------------------------------------------------------------
const int PWMFreq       = 1000;   // 1 kHz carrier
const int PWMResolution = 8;      // 8-bit duty (0–255)
const int channelA      = 0;      // LEDC channel for Motor A
const int channelB      = 1;      // LEDC channel for Motor B

ControllerPtr myController;

// ---------------------------------------------------------------------------
//  Low-level motor driver
//  motor : 0 → Motor A (left)   |   1 → Motor B (right)
//  speed : signed PWM duty  ( + forward,  - reverse,  0 coast )
// ---------------------------------------------------------------------------
void setMotor(int motor, int speed) {
  int pin1, pin2, pwmChannel;

  if (motor == 0) {            // Motor A
    pin1 = AIN1;  pin2 = AIN2;  pwmChannel = channelA;
  } else {                     // Motor B
    pin1 = BIN1;  pin2 = BIN2;  pwmChannel = channelB;
  }

  if (speed > 0) {             // Forward
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    ledcWrite(pwmChannel, speed);
  } else if (speed < 0) {      // Reverse
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    ledcWrite(pwmChannel, abs(speed));
  } else {                     // Coast / stop
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    ledcWrite(pwmChannel, 0);
  }
}

// ---------------------------------------------------------------------------
//  Controller pair / unpair callbacks
// ---------------------------------------------------------------------------
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    Serial.println("CALLBACK: Controller connected");
    myController = ctl;
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("CALLBACK: Controller disconnected");
    myController = nullptr;

    // Fail-safe: gate the H-bridge OFF instantly
    digitalWrite(STBY, LOW);
  }
}

// ---------------------------------------------------------------------------
//  Setup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // 1. Motor direction pins
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // 2. Enable the motor driver
  digitalWrite(STBY, HIGH);

  // 3. Hardware PWM via LEDC
  ledcSetup(channelA, PWMFreq, PWMResolution);
  ledcSetup(channelB, PWMFreq, PWMResolution);
  ledcAttachPin(PWMA, channelA);
  ledcAttachPin(PWMB, channelB);

  // 4. Bluepad32 setup
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.enableVirtualDevice(false);
}

// ---------------------------------------------------------------------------
//  Main loop
// ---------------------------------------------------------------------------
void loop() {
  BP32.update();

  if (myController && myController->isConnected()) {
    processGamepad(myController);
  }

  delay(10);
}

// ---------------------------------------------------------------------------
//  Read sticks → arcade-mix → drive motors
// ---------------------------------------------------------------------------
void processGamepad(ControllerPtr ctl) {
  // Invert Y so stick-up = forward
  int throttle = -(ctl->axisY());
  int steering =   ctl->axisRX();

  // Deadzone — reject idle stick drift
  if (abs(throttle) < 20) throttle = 0;
  if (abs(steering) < 20) steering = 0;

  // Remap Bluepad32 raw [-511, 512] → PWM [-255, 255]
  int mThrottle = map(throttle, -511, 512, -255, 255);
  int mSteering = map(steering, -511, 512, -255, 255);

  // ----- Arcade-drive mixing -----
  // If the bot turns the wrong way, swap the + and - here
  int speedA = mThrottle + mSteering;   // left
  int speedB = mThrottle - mSteering;   // right

  // Clip to PWM-safe range
  speedA = constrain(speedA, -255, 255);
  speedB = constrain(speedB, -255, 255);

  // Drive
  setMotor(0, speedA);
  setMotor(1, speedB);
}
