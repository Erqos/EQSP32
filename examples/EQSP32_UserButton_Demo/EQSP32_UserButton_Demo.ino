/**
 * @file EQSP32_UserButton_Basic.ino
 * @brief Basic demo: read EQSP32 user (BOOT) button when Erqos IoT is disabled,
 *        and control BLE/Wi-Fi LEDs manually.
 *
 * Requirements:
 * - EQSP32Configs::disableErqosIoT MUST be true.
 *
 * Behavior:
 * - On button press: BLE LED ON immediately
 * - If held >= 3 seconds: Wi-Fi LED ON as well
 * - On release: both LEDs OFF
 * - Prints press/release and hold-time info to Serial
 *
 * @author Erqos Technologies
 * @date 2026-01-29
 */

#include <EQSP32.h>

EQSP32 eqsp32;

static bool wasPressed = false;
static uint32_t pressStartMs = 0;
static bool longHoldActivated = false;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\n== EQSP32 User Button Basic Demo ==");

  // IMPORTANT: Disable Erqos IoT core so the button + LEDs are fully user-controlled
  EQSP32Configs cfg;
  cfg.disableErqosIoT = true;

  // (Optional) If you want to keep the device offline entirely, you can also avoid
  // setting Wi-Fi/MQTT configs. This demo doesn't need networking.
  eqsp32.begin(cfg, true);   // verbose logs optional

  // Ensure LEDs start OFF
  eqsp32.resetBleLed();
  eqsp32.resetWifiLed();

  Serial.println("Erqos IoT is disabled -> user button + LEDs are controlled by this sketch.");
  Serial.println("Press BOOT: BLE LED ON, hold 3s: Wi-Fi LED ON, release: both OFF.\n");
}

void loop() {
  const bool pressed = eqsp32.readUserButton();
  const uint32_t now = millis();

  // Rising edge (just pressed)
  if (pressed && !wasPressed) {
    pressStartMs = now;
    longHoldActivated = false;

    eqsp32.setBleLed();      // immediate feedback
    eqsp32.resetWifiLed();   // ensure Wi-Fi LED starts OFF

    Serial.println("Button: PRESSED -> BLE LED ON");
  }

  // While held
  if (pressed) {
    const uint32_t heldMs = now - pressStartMs;

    if (!longHoldActivated && heldMs >= 3000UL) {
      longHoldActivated = true;
      eqsp32.setWifiLed();
      Serial.println("Button: LONG HOLD (>= 3s) -> Wi-Fi LED ON");
    }
  }

  // Falling edge (just released)
  if (!pressed && wasPressed) {
    const uint32_t heldMs = now - pressStartMs;

    eqsp32.resetBleLed();
    eqsp32.resetWifiLed();

    Serial.print("Button: RELEASED (held ");
    Serial.print(heldMs);
    Serial.println(" ms) -> LEDs OFF\n");
  }

  wasPressed = pressed;
  delay(10);  // keep it FreeRTOS-friendly
}
