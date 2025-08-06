/**
 * @file EQSP32_EQXPT_Demo.ino
 * @brief Demonstrates the usage of EQXPT expansion modules for PT100 temperature readings.
 *
 * This example connects two EQXPT modules to the EQSP32 and configures:
 * - 2 PT100 sensors per module.
 * - Reads and logs temperature data from both EQXPT modules.
 * - Detects and reports wiring or sensor errors.
 * - Configures Channel 1 as a 3-wire PT100 sensor.
 * - Configures Channel 2 as a 2/4-wire PT100 sensor.
 *
 * Hardware Setup:
 * - EQXPT Modules connected to EQSP32 (first module is indexed as 1, second as 2).
 * - PT100 sensors connected to each channel.
 * - Channel 1 of both modules is set to 3-wire mode.
 * - Channel 2 of both modules is set to 2/4-wire mode.
 *
  * Detection Behavior:
 * - EQXPT modules are auto-detected during the initial eqsp32.begin() call.
 * - If a module is not connected at boot time, it will not be recognized later.
 *
 * Features:
 * - Reads temperature from PT100 sensors and displays them in Celsius.
 * - Handles and prints simplified error messages.
 * - Supports two EQXPT modules (4 total PT100 sensors).
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include <EQSP32.h>

EQSP32 eqsp32;

#define EQXPT_MODULE_1  1
#define EQXPT_MODULE_2  2

#define LOOP_DELAY_MS 1000

/**
 * @brief Prints a general error message for invalid PT100 readings.
 * @param value The raw sensor value (not used here).
 */
void handlePTSensorError(int value) {
    Serial.print("⚠️ Error\t");
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQSP32 EQXPT PT100 Demo...");

    eqsp32.begin();

    // Configure EQXPT Module 1 if detected
    if (eqsp32.isModuleDetected(EQXPT(EQXPT_MODULE_1))) {
        eqsp32.pinMode(EQXPT(EQXPT_MODULE_1, 1), PT100_3W);   // 3-wire
        eqsp32.pinMode(EQXPT(EQXPT_MODULE_1, 2), PT100_24W);  // 2/4-wire
    } else {
        Serial.println("⚠️ EQXPT Module 1 not detected. Skipping configuration.");
    }

    // Configure EQXPT Module 2 if detected
    if (eqsp32.isModuleDetected(EQXPT(EQXPT_MODULE_2))) {
        eqsp32.pinMode(EQXPT(EQXPT_MODULE_2, 1), PT100_3W);
        eqsp32.pinMode(EQXPT(EQXPT_MODULE_2, 2), PT100_24W);
    } else {
        Serial.println("⚠️ EQXPT Module 2 not detected. Skipping configuration.");
    }
}

void loop() {
    Serial.println("\n========================================");
    Serial.println("📡 Reading PT100 Sensor Data...\n");

    // -------- Module 1 --------
    if (eqsp32.isModuleDetected(EQXPT(EQXPT_MODULE_1))) {
        Serial.print("🌡️ EQXPT Module 1: ");
        for (uint8_t ch = 1; ch <= EQXPT_CHANNELS; ch++) {
            int value = eqsp32.readPin(EQXPT(EQXPT_MODULE_1, ch));
            Serial.printf("(%d) ", ch);

            if (IS_PT_VALID(value)) {
                // Convert raw value (hundredths of °C) to float in Celsius
                Serial.printf("%.1f°C\t", CONVERT_PT(value) );
				// Serial.printf("%.1f°C\t", value / 100.0);	// Same as using CONVERT_PT() 
            } else {
                handlePTSensorError(value);
            }
        }
        Serial.println();
    } else {
        Serial.println("⚠️ EQXPT Module 1 not detected.");
    }

    // -------- Module 2 --------
    if (eqsp32.isModuleDetected(EQXPT(EQXPT_MODULE_2))) {
        Serial.print("🌡️ EQXPT Module 2: ");
        for (uint8_t ch = 1; ch <= EQXPT_CHANNELS; ch++) {
            int value = eqsp32.readPin(EQXPT(EQXPT_MODULE_2, ch));
            Serial.printf("(%d) ", ch);

            if (IS_PT_VALID(value)) {
                // Convert raw value (hundredths of °C) to float in Celsius
                Serial.printf("%.1f°C\t", CONVERT_PT(value) );
				// Serial.printf("%.1f°C\t", value / 100.0);	// Same as using CONVERT_PT() 
            } else {
                handlePTSensorError(value);
            }
        }
        Serial.println();
    } else {
        Serial.println("⚠️ EQXPT Module 2 not detected.");
    }

    delay(LOOP_DELAY_MS);
}