/**
 * @file EQSP32_EQXIO_Demo.ino
 * @brief Demonstrates EQXIO input/output modes: DIN, SWT, POUT, and RELAY.
 *
 * This example demonstrates how to:
 * - Detect an EQXIO module connected to the EQSP32 system.
 * - Configure an EQXIO input pin as DIN or SWT (debounced input).
 * - Use trigger modes (STATE / ON_RISING / ON_FALLING / ON_TOGGLE) on DIN/SWT reads.
 * - Configure an EQXIO output pin as POUT (PWM) or RELAY (start power -> hold power).
 *
 * Requirements:
 * - EQXIO module connected during eqsp32.begin() (modules are detected during begin()).
 * - Button wired to EQXIO Pin 1.
 * - LED / load wired to EQXIO Pin 2.
 *
 * Hardware Setup:
 * - Button -> EQXIO Pin 1 (Input)
 * - LED/Load -> EQXIO Pin 2 (Output)
 *
 * Author: Erqos Technologies
 * Date: 2026-01-29
 */

#include <EQSP32.h>

EQSP32 eqsp32;

// Define EQXIO module index (per module type, 1-based)
#define EQXIO_INDEX  1

// EQXIO Pin Assignments
#define EQXIO_BUTTON_PIN  EQXIO(EQXIO_INDEX, EQXIO_PIN_1)
#define EQXIO_OUTPUT_PIN  EQXIO(EQXIO_INDEX, EQXIO_PIN_2)

#define LOOP_DELAY_MS 100

// ---------------------------
// Demo selection switches
// ---------------------------

// Input demo: choose ONE
#define DEMO_USE_SWT_INPUT     1   // 0 = DIN, 1 = SWT (debounced)

// Output demo: choose ONE
#define DEMO_USE_RELAY_OUTPUT  1   // 0 = POUT, 1 = RELAY

// Trigger mode for DIN/SWT reads
#define INPUT_TRIGGER_MODE     ON_RISING   // try: STATE / ON_RISING / ON_FALLING / ON_TOGGLE

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting EQSP32 EQXIO Demo...");

  eqsp32.begin();  // default init

  // Ensure module exists (detected during begin)
  while (!eqsp32.isModuleDetected(EQXIO(EQXIO_INDEX))) {
    Serial.println("❌ EQXIO not detected. Connect EQXIO and reboot.");
    delay(1000);
  }
  Serial.println("✅ EQXIO module detected!");

  // ---------------------------
  // Configure INPUT pin
  // ---------------------------
#if DEMO_USE_SWT_INPUT
  eqsp32.pinMode(EQXIO_BUTTON_PIN, SWT);
  eqsp32.configSWT(EQXIO_BUTTON_PIN, 100);   // debounce time in ms
  Serial.println("Input mode: SWT (debounced)");
#else
  eqsp32.pinMode(EQXIO_BUTTON_PIN, DIN);
  Serial.println("Input mode: DIN");
#endif

  // ---------------------------
  // Configure OUTPUT pin
  // ---------------------------
#if DEMO_USE_RELAY_OUTPUT
  eqsp32.pinMode(EQXIO_OUTPUT_PIN, RELAY);
  eqsp32.configRELAY(EQXIO_OUTPUT_PIN, 300, 1500);  // holdValue=300 (30%), derateDelay=1500ms
  Serial.println("Output mode: RELAY (start->hold)");
#else
  eqsp32.pinMode(EQXIO_OUTPUT_PIN, POUT);
  Serial.println("Output mode: POUT (PWM)");
#endif

  // Start output OFF
  eqsp32.pinValue(EQXIO_OUTPUT_PIN, 0);
}

void loop() {
  // If your system supports hot-unplug detection, you can re-check:
  if (!eqsp32.isModuleDetected(EQXIO(EQXIO_INDEX))) {
    Serial.println("⚠️ EQXIO module disconnected!");
    delay(500);
    return;
  }

  // Trigger-style read: fires depending on INPUT_TRIGGER_MODE
  bool trig = (eqsp32.readPin(EQXIO_BUTTON_PIN, INPUT_TRIGGER_MODE) > 0);

  // For STATE mode, trig == current state.
  // For ON_RISING/ON_FALLING/ON_TOGGLE, trig is an event (typically 1 only on the transition).
  if (trig) {
    Serial.println("\n========================================");
    Serial.print("🔘 Input trigger fired (mode = ");
    switch (INPUT_TRIGGER_MODE) {
      case STATE:      Serial.print("STATE"); break;
      case ON_RISING:  Serial.print("ON_RISING"); break;
      case ON_FALLING: Serial.print("ON_FALLING"); break;
      case ON_TOGGLE:  Serial.print("ON_TOGGLE"); break;
    }
    Serial.println(")");

    // Demonstration action:
    // - POUT: toggle between OFF and 100%
    // - RELAY: start cycle with 1000 (full power), then it will drop to holdValue after derateDelay
    static bool isOn = false;
    isOn = !isOn;

#if DEMO_USE_RELAY_OUTPUT
    if (isOn) {
      Serial.println("🔁 RELAY cycle: START (100%) -> will auto-derate to HOLD");
      eqsp32.pinValue(EQXIO_OUTPUT_PIN, 1000); // start power
    } else {
      Serial.println("🛑 RELAY OFF (reset cycle)");
      eqsp32.pinValue(EQXIO_OUTPUT_PIN, 0);    // must be 0 to restart cycle next time
    }
#else
    int out = isOn ? 1000 : 0;
    Serial.print("💡 POUT set to: ");
    Serial.println(out);
    eqsp32.pinValue(EQXIO_OUTPUT_PIN, out);
#endif

    // Print modes (should remain what we configured)
    Serial.print("📌 Input pin mode: ");
    Serial.println(eqsp32.readMode(EQXIO_BUTTON_PIN));

    Serial.print("📌 Output pin mode: ");
    Serial.println(eqsp32.readMode(EQXIO_OUTPUT_PIN));
  }

  delay(LOOP_DELAY_MS);
}
