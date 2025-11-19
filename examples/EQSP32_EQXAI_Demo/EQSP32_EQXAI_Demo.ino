/**
 * @file EQSP32_EQXAI_Demo.ino
 * @brief Demonstrates the usage of AIN (Analog Voltage Input) and CIN (Analog Current Input) modes on the EQXAI expansion module.
 *
 * This example assumes:
 *   - EQXAI module index 1 is connected to the EQSP32 expansion port.
 *   - EQXAI channel 1 is used as AIN (voltage input).
 *   - EQXAI channel 2 is used as CIN (4–20 mA current input).
 *
 * Mode Verification:
 *   - As with the main-unit AIN/CIN demo, this example uses `readMode()` to confirm
 *     that the requested mode (AIN or CIN) is actually applied on the EQXAI channels.
 *
 * Conversion:
 *   - AIN values are returned in millivolts. Use `CONVERT_AIN()` to convert to volts.
 *   - CIN values are returned in hundredths of milliamps (mA). Use `CONVERT_CIN()` to convert to mA.
 *
 * Important:
 *   - CIN mode is intended for 4–20 mA industrial sensors connected to the EQXAI current-input channels.
 *   - If an overcurrent condition (>21 mA) is detected, `readPin()` will return `CIN_OC_ERROR` (-1).
 *   - Always check for `CIN_OC_ERROR` before performing conversion on CIN values.
 *
 * @author Erqos Technologies
 * @date 2025-11-18
 */

#include <EQSP32.h>

// Create EQSP32 instance
EQSP32 eqsp32;

// EQXAI module index (logical index on the EQSP32 expansion bus)
#define EQXAI_MODULE_INDEX  1

// Assign EQXAI channels for AIN and CIN
// Configure channel 1 as AIN, channel 2 as CIN on EQXAI module 1.
const uint32_t eqxaiAinPin = EQXAI(EQXAI_MODULE_INDEX, 1);
const uint32_t eqxaiCinPin = EQXAI(EQXAI_MODULE_INDEX, 2);

void setup() {
    Serial.begin(115200);
    delay(200);

    eqsp32.begin();

    Serial.println("\n🚀 Starting EQSP32 EQXAI Demo ...\n");

    // Detect EQXAI module 1
    Serial.println("Checking for EQXAI module...");
    if (!eqsp32.isModuleDetected(EQXAI(EQXAI_MODULE_INDEX))) {
        Serial.printf("❌ EQXAI module %d NOT detected. Please connect an EQXAI module and reboot.\n", EQXAI_MODULE_INDEX);
        // Block here if module not detected
        while (true) {
            delay(1000);
        }
    }
    Serial.printf("✅ EQXAI module %d detected.\n\n", EQXAI_MODULE_INDEX);

    // Configure EQXAI channels
    Serial.println("Configuring EQXAI channels:");
    Serial.println(" - Channel 1 as AIN (voltage)");
    Serial.println(" - Channel 2 as CIN (4–20 mA current)");

    eqsp32.pinMode(eqxaiAinPin, AIN);
    eqsp32.pinMode(eqxaiCinPin, CIN);

    // Optional: read back modes for verification
    EQ_PinMode ainMode = eqsp32.readMode(eqxaiAinPin);
    EQ_PinMode cinMode = eqsp32.readMode(eqxaiCinPin);

    Serial.printf("EQXAI ch1 requested: AIN, active mode: %d (%s)\n", ainMode, ainMode==AIN ? "AIN" : "NOT AIN");
    Serial.printf("EQXAI ch2 requested: CIN, active mode: %d (%s)\n\n", cinMode, cinMode==CIN ? "CIN" : "NOT CIN");

    delay(100);   // Wait for EQXAI updated values after mode change
}

void loop() {
    static bool disconnectionDetected = false;

    if (!eqsp32.isModuleDetected(EQXAI(EQXAI_MODULE_INDEX))) {
        disconnectionDetected = true;
        Serial.printf("EQXAI module has been disconnected.\n");
    } else if (disconnectionDetected) {
        disconnectionDetected = false;

        // Reconfigure EQXAI when reconnected
        eqsp32.pinMode(eqxaiAinPin, AIN);
        eqsp32.pinMode(eqxaiCinPin, CIN);
    }

    // --- AIN Mode Reading (Voltage on EQXAI ch1) ---
    int ain_raw = eqsp32.readPin(eqxaiAinPin);
    float ain_v = CONVERT_AIN(ain_raw); // Same as: ain_raw / 1000.0

    Serial.print("EQXAI AIN  (Module ");
    Serial.print(EQXAI_MODULE_INDEX);
    Serial.print(", Ch 1): ");
    Serial.print(ain_v, 3);
    Serial.println(" V");

    // --- CIN Mode Reading (4–20 mA Sensor on EQXAI ch2) ---
    if (eqsp32.readMode(eqxaiCinPin) == CIN) {
        int cin_raw = eqsp32.readPin(eqxaiCinPin);

        if (cin_raw == CIN_OC_ERROR) {
            Serial.print("EQXAI CIN  (Module ");
            Serial.print(EQXAI_MODULE_INDEX);
            Serial.println(", Ch 2): ⚠️ Overcurrent Detected (>21 mA)");
        } else {
            float cin_ma = CONVERT_CIN(cin_raw); // Same as: cin_raw / 100.0

            Serial.print("EQXAI CIN  (Module ");
            Serial.print(EQXAI_MODULE_INDEX);
            Serial.print(", Ch 2): ");
            Serial.print(cin_ma, 2);
            Serial.println(" mA");
        }
    } else {
        Serial.print("EQXAI CIN  (Module ");
        Serial.print(EQXAI_MODULE_INDEX);
        Serial.println(", Ch 2): Not in CIN mode.");
    }

    Serial.println("\n-----------------------------\n");
    delay(2000);  // Simple periodic update
}
