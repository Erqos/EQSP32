/**
 * @file EQSP32_Analog_Inputs_AIN_CIN_Modes_Demo.ino
 * @brief Demonstrates the usage of AIN (Analog Voltage Input) and CIN (Analog Current Input) modes on EQSP32.
 *
 * EQSP32 pins 1 to 8 support both AIN and CIN modes.
 * This example configures:
 *   - One pin (PIN 1) as AIN to read a voltage signal (in millivolts)
 *   - One pin (PIN 2) as CIN to read a 4–20 mA current signal (in hundredths of milliamps)
 *
 * Mode Verification:
 *   - To ensure reliable configuration, this demo uses `readMode()` to confirm the actual mode applied to each pin.
 *   - On hardware versions that do not support CIN mode, the pin will silently fallback to AIN mode.
 *   - By printing the result of `readMode()`, the example provides insight into what mode is truly active.
 *
 * Conversion:
 *   - AIN values are returned in millivolts. To convert to volts, divide by 1000 or use `CONVERT_AIN()`.
 *   - CIN values are returned in hundredths of milliamps (mA). To convert to mA, divide by 100 or use `CONVERT_CIN()`.
 *
 * Important:
 *   - CIN mode is intended for 4–20 mA industrial sensors.
 *   - If an overcurrent condition (>21 mA) is detected, the protection mechanism will be triggered,
 *     and `readPin()` will return `CIN_OC_ERROR` (-1).
 *   - Always check for `CIN_OC_ERROR` before performing conversion on CIN values.
 */


#include <EQSP32.h>

// Create EQSP32 instance
EQSP32 eqsp32;

// Assign analog pins (all pins 1–8 support AIN and CIN)
const int ainPin = EQ_PIN_1;
const int cinPin = EQ_PIN_2;

void setup() {
    Serial.begin(115200);
    eqsp32.begin();

    Serial.println("\nEQSP32 AIN & CIN Mode Demo\n");

    // Configure pins
    eqsp32.pinMode(ainPin, AIN);
    eqsp32.pinMode(cinPin, CIN);
}

void loop() {
    // --- AIN Mode Reading (Voltage) ---
    int ain_raw = eqsp32.readPin(ainPin);
    float ain_v = CONVERT_AIN(ain_raw); // Same as: ain_raw / 1000.0

    Serial.print("AIN  (Pin ");
    Serial.print(ainPin);
    Serial.print("): ");
    Serial.print(ain_v);
    Serial.println(" V");

    // --- CIN Mode Reading (4–20 mA Sensor) ---
    if (eqsp32.readMode(cinPin) == CIN) {
      int cin_raw = eqsp32.readPin(cinPin);

      if (cin_raw == CIN_OC_ERROR) {
          Serial.print("CIN  (Pin ");
          Serial.print(cinPin);
          Serial.println("): ⚠️ Overcurrent Detected (>21mA)");
      } else {
          float cin_ma = CONVERT_CIN(cin_raw); // Same as: cin_raw / 100.0

          Serial.print("CIN  (Pin ");
          Serial.print(cinPin);
          Serial.print("): ");
          Serial.print(cin_ma);
          Serial.println(" mA");
      }
    } else {
        Serial.print("CIN  (Pin ");
        Serial.print(cinPin);
        Serial.println("): Not in CIN mode - verify that your current hardware supports CIN mode.");
    }


    Serial.println("\n-----------------------------\n");
    delay(2000);
}