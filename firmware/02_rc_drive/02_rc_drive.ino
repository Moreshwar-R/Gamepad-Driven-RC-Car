#include <Bluepad32.h>

// ---------------------------------------------------------------------------
//  Pin Map — ESP32 → BTS7960 ×2 (canonical dual-PWM interface)
// ---------------------------------------------------------------------------
#define L_RPWM  12   // Left driver  — forward PWM
#define L_LPWM  14   // Left driver  — reverse PWM
#define R_RPWM  13   // Right driver — forward PWM
#define R_LPWM  26   // Right driver — reverse PWM
#define EN      33   // Tied to L_EN + R_EN on BOTH drivers
                     //   HIGH = bridges enabled,  LOW = full coast / fail-safe

// ---------------------------------------------------------------------------
//  PWM Configuration
//  20 kHz keeps the BTS7960 above the audible band (no whine) and well
//  inside its 25 kHz max switching spec.
// ---------------------------------------------------------------------------
const int PWMFreq       = 20000;  // 20 kHz carrier
const int PWMResolution = 8;      // 8-bit duty (0–255)

// LEDC channel assignment — one per PWM pin (4 total)
const int chL_R = 0;   // Left  RPWM
const int chL_L = 1;   // Left  LPWM
const int chR_R = 2;   // Right RPWM
const int chR_L = 3;   // Right LPWM

ControllerPtr myController;

// ---------------------------------------------------------------------------
//  Low-level motor driver
//  motor : 0 → left   |   1 → right
//  speed : signed PWM duty  ( + forward,  - reverse,  0 brake )
// ---------------------------------------------------------------------------
void setMotor(int motor, int speed) {
  int rpwmCh, lpwmCh;

  if (motor == 0) {              // Left driver
    rpwmCh = chL_R;  lpwmCh = chL_L;
  } else {                       // Right driver
    rpwmCh = chR_R;  lpwmCh = chR_L;
  }

  speed = constrain(speed, -255, 255);

  if (speed > 0) {               // Forward: drive RPWM, hold LPWM low
    ledcWrite(rpwmCh, speed);
    ledcWrite(lpwmCh, 0);
  } else if (speed < 0) {        // Reverse: drive LPWM, hold RPWM low
    ledcWrite(rpwmCh, 0);
    ledcWrite(lpwmCh, -speed);
  } else {                       // Brake (both low while EN stays HIGH)
    ledcWrite(rpwmCh, 0);
    ledcWrite(lpwmCh, 0);
  }
}

// ---------------------------------------------------------------------------
//  Controller pair / unpair callbacks
// ---------------------------------------------------------------------------
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    Serial.println("CALLBACK: Controller connected");
    myController = ctl;

    // Re-enable bridges now that we have a valid command source
    digitalWrite(EN, HIGH);
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("CALLBACK: Controller disconnected");
    myController = nullptr;

    // Fail-safe: gate both BTS7960 bridges OFF instantly
    digitalWrite(EN, LOW);
  }
}

// ---------------------------------------------------------------------------
//  Setup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // 1. Enable line — start LOW until controller is paired
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);

  // 2. Hardware PWM via LEDC — attach all 4 PWM pins
  ledcSetup(chL_R, PWMFreq, PWMResolution);
  ledcSetup(chL_L, PWMFreq, PWMResolution);
  ledcSetup(chR_R, PWMFreq, PWMResolution);
  ledcSetup(chR_L, PWMFreq, PWMResolution);
  ledcAttachPin(L_RPWM, chL_R);
  ledcAttachPin(L_LPWM, chL_L);
  ledcAttachPin(R_RPWM, chR_R);
  ledcAttachPin(R_LPWM, chR_L);

  // 3. Make sure everything starts at zero duty
  ledcWrite(chL_R, 0);
  ledcWrite(chL_L, 0);
  ledcWrite(chR_R, 0);
  ledcWrite(chR_L, 0);

  // 4. Bluepad32 setup — bridges stay disabled until a controller pairs
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
  int speedL = mThrottle + mSteering;   // left
  int speedR = mThrottle - mSteering;   // right

  // Clip to PWM-safe range
  speedL = constrain(speedL, -255, 255);
  speedR = constrain(speedR, -255, 255);

  // Drive
  setMotor(0, speedL);
  setMotor(1, speedR);
}
