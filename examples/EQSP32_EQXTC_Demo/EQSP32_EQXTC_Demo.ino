/**
 * @file EQSP32_EQXTC_Demo.ino
 * @brief Demonstrates the usage of EQXTC expansion modules for thermocouple temperature readings.
 *
 * This example connects two EQXTC modules to the EQSP32 and configures:
 * - Up to 6 thermocouple sensors per module.
 * - Reads and logs temperature data from both EQXTC modules.
 * - Detects and reports sensor errors (open circuit, short to ground, short to VCC).
 *
 * Hardware Setup:
 * - EQXTC Modules connected to EQSP32 (first module is indexed as 1, second as 2).
 * - Up to 6 thermocouples connected to each EQXTC module.
 *
 * Features:
 * - Reads temperature from thermocouples and displays them in Celsius.
 * - Handles and prints sensor error messages.
 * - Supports multiple EQXTC modules (up to 2 in this demo).
 *
 * @author Erqos Technologies
 * @date 2025-02-20
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
        case TC_FAULT_OPEN:
            Serial.print("⚠️ OPEN\t");
            break;
        case TC_FAULT_SHORT_GND:
            Serial.print("⚠️ GND\t");
            break;
        case TC_FAULT_SHORT_VCC:
            Serial.print("⚠️ VCC\t");
            break;
        default:
            Serial.print("❌ ERR\t");
            break;
    }
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\nStarting EQSP32 EQXTC Demo...");

    // Initialize EQSP32
    eqsp32.begin();  // Default initialization
}

void loop() {
    // Print a divider for better readability
    Serial.println("\n========================================");
    Serial.println("📡 Reading Thermocouple Data...\n");

    // Read thermocouples from Module 1
    Serial.print("🌡️ EQXTC Module 1: ");
    for (uint8_t channel = 1; channel <= EQXTC_CHANNELS; channel++) {
        int value = eqsp32.readPin(EQXTC(EQXTC_MODULE_1, channel));

        Serial.printf("(%d) ", channel);

        if (IS_TC_VALID(value)) {
            Serial.printf("%.1f°C\t", value / 10.0); // Print valid temperature with one decimal
        } else {
            handleThermocoupleError(value); // Handle and print error
        }
    }
    Serial.println(); // New line after module 1 readings

    // Read thermocouples from Module 2
    Serial.print("🌡️ EQXTC Module 2: ");
    for (uint8_t channel = 1; channel <= EQXTC_CHANNELS; channel++) {
        int value = eqsp32.readPin(EQXTC(EQXTC_MODULE_2, channel));

        Serial.printf("(%d) ", channel);

        if (IS_TC_VALID(value)) {
            Serial.printf("%.1f°C\t", value / 10.0); // Print valid temperature with one decimal
        } else {
            handleThermocoupleError(value); // Handle and print error
        }
    }
    Serial.println(); // New line after module 2 readings

    // Add a delay before next reading
    delay(LOOP_DELAY_MS);
}
