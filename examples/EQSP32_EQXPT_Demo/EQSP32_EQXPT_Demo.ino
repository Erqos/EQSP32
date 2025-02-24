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
 * Features:
 * - Reads temperature from PT100 sensors and displays them in Celsius.
 * - Handles and prints simplified error messages.
 * - Supports two EQXPT modules (4 total PT100 sensors).
 *
 * @author Erqos Technologies
 * @date 2025-02-20
 */

#include <EQSP32.h>  // Include the EQSP32 library

EQSP32 eqsp32;

// Define EQXPT module indices (first module starts from 1)
#define EQXPT_MODULE_1  1
#define EQXPT_MODULE_2  2

// Loop Delay
#define LOOP_DELAY_MS 1000  

/**
 * @brief Handles PT100 sensor errors and prints a generic message.
 * @param value The error code.
 */
void handlePTSensorError(int value) {
    Serial.print("⚠️ Wiring or sensor error\t");
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\nStarting EQSP32 EQXPT PT100 Demo...");

    // Initialize EQSP32
    eqsp32.begin();  // Default initialization

    // Configure PT100 sensor types
    eqsp32.pinMode(EQXPT(EQXPT_MODULE_1, 1), PT100_3W);  // Module 1, Channel 1 → 3-wire mode
    eqsp32.pinMode(EQXPT(EQXPT_MODULE_1, 2), PT100_24W);  // Module 1, Channel 2 → 2/4-wire mode

    eqsp32.pinMode(EQXPT(EQXPT_MODULE_2, 1), PT100_3W);  // Module 2, Channel 1 → 3-wire mode
    eqsp32.pinMode(EQXPT(EQXPT_MODULE_2, 2), PT100_24W);  // Module 2, Channel 2 → 2/4-wire mode
}

void loop() {
    // Print a divider for better readability
    Serial.println("\n========================================");
    Serial.println("📡 Reading PT100 Sensor Data...\n");

    // Read PT100 sensors from Module 1
    Serial.print("🌡️ EQXPT Module 1: ");
    for (uint8_t channel = 1; channel <= EQXPT_CHANNELS; channel++) {
        int value = eqsp32.readPin(EQXPT(EQXPT_MODULE_1, channel));

        Serial.printf("(%d) ", channel);

        if (IS_PT_VALID(value)) {
            Serial.printf("%.1f°C\t", value / 10.0); // Print valid temperature with one decimal
        } else {
            Serial.print("⚠️ Error\t");
        }
    }
    Serial.println(); // New line after module 1 readings

    // Read PT100 sensors from Module 2
    Serial.print("🌡️ EQXPT Module 2: ");
    for (uint8_t channel = 1; channel <= EQXPT_CHANNELS; channel++) {
        int value = eqsp32.readPin(EQXPT(EQXPT_MODULE_2, channel));

        Serial.printf("(%d) ", channel);

        if (IS_PT_VALID(value)) {
            Serial.printf("%.1f°C\t", value / 10.0); // Print valid temperature with one decimal
        } else {
            Serial.print("⚠️ Error\t");
        }
    }
    Serial.println(); // New line after module 2 readings

    // Add a delay before next reading
    delay(LOOP_DELAY_MS);
}
