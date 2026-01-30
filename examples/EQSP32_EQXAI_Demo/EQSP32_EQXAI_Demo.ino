/**
 * @file EQSP32_EQXAI_Demo.ino
 * @brief Demonstrates AIN (Analog Voltage), CIN (4–20 mA Current), and TIN (Temperature Input) modes on the EQXAI expansion module.
 *
 * This example assumes:
 *   - EQXAI module index 1 is connected to the EQSP32 expansion port.
 *   - EQXAI channel 1 is used as AIN (voltage input).
 *   - EQXAI channel 2 is used as CIN (4–20 mA current input).
 *   - EQXAI channel 3 is used as TIN (NTC temperature input).
 *
 * Mode Verification:
 *   - Uses `readMode()` to confirm that the requested mode is actually applied.
 *
 * Conversions:
 *   - AIN values are returned in millivolts. Use `CONVERT_AIN()` to convert to volts.
 *   - CIN values are returned in hundredths of milliamps. Use `CONVERT_CIN()` to convert to mA.
 *   - TIN values are returned in Celsius * 10. Use `CONVERT_TIN()` to convert to °C.
 *
 * CIN Safety:
 *   - If an overcurrent condition (>21 mA) is detected, `readPin()` returns `CIN_OC_ERROR` (-1).
 *
 * TIN Validity:
 *   - TIN may return error codes:
 *       - `TIN_OPEN_CIRCUIT`  (-9999)
 *       - `TIN_SHORT_CIRCUIT` (9999)
 *   - Always check `IS_TIN_VALID(value)` before converting.
 *
 * @author Erqos Technologies
 * @date 2026-01-29
 */

#include <EQSP32.h>

// Create EQSP32 instance
EQSP32 eqsp32;

// EQXAI module index (logical index on the EQSP32 expansion bus)
#define EQXAI_MODULE_INDEX  1

// Assign EQXAI channels for AIN, CIN, and TIN
const uint32_t eqxaiAinPin = EQXAI(EQXAI_MODULE_INDEX, 1);  // Ch1: AIN
const uint32_t eqxaiCinPin = EQXAI(EQXAI_MODULE_INDEX, 2);  // Ch2: CIN
const uint32_t eqxaiTinPin = EQXAI(EQXAI_MODULE_INDEX, 3);  // Ch3: TIN

static void printTinValue(int tin_raw) {
    if (!IS_TIN_VALID(tin_raw)) {
        if (tin_raw == TIN_OPEN_CIRCUIT) {
            Serial.println("⚠️ TIN: Open circuit detected");
        } else if (tin_raw == TIN_SHORT_CIRCUIT) {
            Serial.println("⚠️ TIN: Short circuit detected");
        } else {
            Serial.print("⚠️ TIN: Invalid value: ");
            Serial.println(tin_raw);
        }
        return;
    }

    float tempC = CONVERT_TIN(tin_raw); // tin_raw / 10.0
    Serial.print("🌡️ TIN: ");
    Serial.print(tempC, 1);
    Serial.println(" °C");
}

void setup() {
    Serial.begin(115200);
    delay(200);

    eqsp32.begin();

    Serial.println("\n🚀 Starting EQSP32 EQXAI Demo (AIN + CIN + TIN) ...\n");

    // Detect EQXAI module
    Serial.println("Checking for EQXAI module...");
    if (!eqsp32.isModuleDetected(EQXAI(EQXAI_MODULE_INDEX))) {
        Serial.printf("❌ EQXAI module %d NOT detected. Please connect an EQXAI module and reboot.\n", EQXAI_MODULE_INDEX);
        while (true) delay(1000);
    }
    Serial.printf("✅ EQXAI module %d detected.\n\n", EQXAI_MODULE_INDEX);

    // Configure EQXAI channels
    Serial.println("Configuring EQXAI channels:");
    Serial.println(" - Channel 1 as AIN (voltage)");
    Serial.println(" - Channel 2 as CIN (4–20 mA current)");
    Serial.println(" - Channel 3 as TIN (temperature)");

    eqsp32.pinMode(eqxaiAinPin, AIN);
    eqsp32.pinMode(eqxaiCinPin, CIN);
    eqsp32.pinMode(eqxaiTinPin, TIN);

    // Optional: configure TIN parameters (beta + reference resistance)
    // If EQXAI uses the same TIN model as the main unit, this applies.
    // If not needed, remove it.
    eqsp32.configTIN(eqxaiTinPin, 3435, 10000);

    // Verify applied modes
    EQ_PinMode ainMode = eqsp32.readMode(eqxaiAinPin);
    EQ_PinMode cinMode = eqsp32.readMode(eqxaiCinPin);
    EQ_PinMode tinMode = eqsp32.readMode(eqxaiTinPin);

    Serial.printf("EQXAI ch1 requested: AIN, active mode: %d (%s)\n", ainMode, ainMode == AIN ? "AIN" : "NOT AIN");
    Serial.printf("EQXAI ch2 requested: CIN, active mode: %d (%s)\n", cinMode, cinMode == CIN ? "CIN" : "NOT CIN");
    Serial.printf("EQXAI ch3 requested: TIN, active mode: %d (%s)\n\n", tinMode, tinMode == TIN ? "TIN" : "NOT TIN");

    delay(200);
}

void loop() {
    static bool disconnectionDetected = false;

    // Handle disconnect/reconnect
    if (!eqsp32.isModuleDetected(EQXAI(EQXAI_MODULE_INDEX))) {
        if (!disconnectionDetected) {
            disconnectionDetected = true;
            Serial.println("⚠️ EQXAI module disconnected!");
        }
        delay(1000);
        return;
    } else if (disconnectionDetected) {
        disconnectionDetected = false;
        Serial.println("✅ EQXAI module reconnected. Reconfiguring modes...");

        eqsp32.pinMode(eqxaiAinPin, AIN);
        eqsp32.pinMode(eqxaiCinPin, CIN);
        eqsp32.pinMode(eqxaiTinPin, TIN);
        eqsp32.configTIN(eqxaiTinPin, 3435, 10000);

        delay(200);
    }

    Serial.println("\n-----------------------------");

    // --- AIN (Voltage) ---
    int ain_raw = eqsp32.readPin(eqxaiAinPin);
    float ain_v = CONVERT_AIN(ain_raw); // mV -> V

    Serial.print("🔌 AIN: ");
    Serial.print(ain_v, 3);
    Serial.println(" V");

    // --- CIN (Current) ---
    int cin_raw = eqsp32.readPin(eqxaiCinPin);
    if (cin_raw == CIN_OC_ERROR) {
        Serial.println("⚠️ CIN: Overcurrent detected (>21 mA)");
    } else {
        float cin_ma = CONVERT_CIN(cin_raw); // (mA * 100) -> mA
        Serial.print("⚡ CIN: ");
        Serial.print(cin_ma, 2);
        Serial.println(" mA");
    }

    // --- TIN (Temperature) ---
    int tin_raw = eqsp32.readPin(eqxaiTinPin);
    printTinValue(tin_raw);

    delay(2000);
}
