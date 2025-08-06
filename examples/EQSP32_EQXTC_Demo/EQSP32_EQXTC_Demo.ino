 /**
 * @file EQSP32_EQXTC_Demo.ino
 * @brief Demonstrates the usage of EQXTC expansion modules for thermocouple temperature readings.
 *
 * This example connects two EQXTC modules to the EQSP32 and reads temperature data from up to 12 thermocouples.
 *
 * Functional Overview:
 * - Up to 6 thermocouples can be connected to each EQXTC module.
 * - Reads and logs temperature data from both EQXTC modules.
 * - Detects and reports common sensor faults (open circuit, short to GND, or VCC).
 *
 * Hardware Setup:
 * - EQXTC Modules connected to the EQSP32 expansion ports.
 * - First EQXTC module is indexed as 1, second as 2.
 * - Thermocouples connected to available channels on each module.
 *
 * Detection Behavior:
 * - EQXTC modules are auto-detected during the initial eqsp32.begin() call.
 * - If a module is not connected at boot time, it will not be recognized later.
 *
 * Features:
 * - Displays thermocouple readings in degrees Celsius.
 * - Handles and prints error messages using standard fault codes.
 * - Supports multiple EQXTC modules in parallel (up to 2 in this demo).
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */


#include <EQSP32.h>  // Include the EQSP32 library

EQSP32 eqsp32;

// Define EQXTC module indices (first module starts from 1)
#define EQXTC_MODULE_1  1
#define EQXTC_MODULE_2  2

// Loop Delay
#define LOOP_DELAY_MS 1000

/**
 * @brief Handles thermocouple error messages and prints them to Serial.
 * @param value The error code.
 */
void handleThermocoupleError(int value) {
    switch (value) {
        case TC_FAULT_OPEN:      Serial.print("⚠️ OPEN\t"); break;
        case TC_FAULT_SHORT_GND: Serial.print("⚠️ GND\t");  break;
        case TC_FAULT_SHORT_VCC: Serial.print("⚠️ VCC\t");  break;
        default:                 Serial.print("❌ ERR\t");  break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQSP32 EQXTC Demo...");

    eqsp32.begin();
}

void loop() {
    Serial.println("\n========================================");
    Serial.println("📡 Reading Thermocouple Data...\n");

	/*
	* For each thermocouple channel:
	* - Check if the reading is valid IS_TC_VALID()
	* - Convert raw sensor value (in tenths of °C) to float °C using CONVERT_TC()
	* - Display result
	*/


    // -------- Module 1 --------
    if (eqsp32.isModuleDetected(EQXTC(EQXTC_MODULE_1))) {
        Serial.print("🌡️ EQXTC Module 1: ");
        for (uint8_t ch = 1; ch <= EQXTC_CHANNELS; ch++) {
            int value = eqsp32.readPin(EQXTC(EQXTC_MODULE_1, ch));
            Serial.printf("(%d) ", ch);

            if (IS_TC_VALID(value)) {
				Serial.printf("%.1f°C\t", CONVERT_TC(value));
                // Serial.printf("%.1f°C\t", value / 10.0);		// Same as doing CONVERT_TC()
            } else {
                handleThermocoupleError(value);
            }
        }
        Serial.println();
    } else {
        Serial.println("❌ EQXTC Module 1 not detected.");
    }

    // -------- Module 2 --------
    if (eqsp32.isModuleDetected(EQXTC(EQXTC_MODULE_2))) {
        Serial.print("🌡️ EQXTC Module 2: ");
        for (uint8_t ch = 1; ch <= EQXTC_CHANNELS; ch++) {
            int value = eqsp32.readPin(EQXTC(EQXTC_MODULE_2, ch));
            Serial.printf("(%d) ", ch);

            if (IS_TC_VALID(value)) {
                Serial.printf("%.1f°C\t", CONVERT_TC(value));
                // Serial.printf("%.1f°C\t", value / 10.0);		// Same as doing CONVERT_TC()
            } else {
                handleThermocoupleError(value);
            }
        }
        Serial.println();
    } else {
        Serial.println("❌ EQXTC Module 2 not detected.");
    }

    delay(LOOP_DELAY_MS);
}
