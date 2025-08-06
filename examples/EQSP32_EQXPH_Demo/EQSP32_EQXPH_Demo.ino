/**
 * @file EQSP32_EQXPH_Demo.ino
 * @brief Demonstrates the usage of the EQXPH expansion module for pH monitoring.
 *
 * This example connects a single EQXPH module to the EQSP32 and:
 * - Reads the pH value from the onboard sensor channel.
 * - Converts and displays the value in standard pH units (0–14).
 *
 * Hardware Setup:
 * - EQXPH Module connected to EQSP32 (indexed as 1).
 * - BNC pH probe connected to the EQXPH module.
 * 
  * Detection Behavior:
 * - EQXPH modules are auto-detected during the initial eqsp32.begin() call.
 * - If a module is not connected at boot time, it will not be recognized later.
 *
 * Features:
 * - Reads EQXPH module and converts (hundredths of pH) to pH sensor data.
 * - Displays the pH value with two decimal precision.
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include <EQSP32.h>

EQSP32 eqsp32;

#define EQXPH_MODULE    1
#define LOOP_DELAY_MS   1000


void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQXPH pH Monitoring Demo...");

    eqsp32.begin();

    if (!eqsp32.isModuleDetected(EQXPH(EQXPH_MODULE))) {
        Serial.println("⚠️ EQXPH Module not detected. Please check the connection.");
    }
}

void loop() {
    Serial.println("\n========================================");
    Serial.println("📡 Reading pH Sensor Data...\n");

    if (eqsp32.isModuleDetected(EQXPH(EQXPH_MODULE))) {
        int value = eqsp32.readPin(EQXPH(EQXPH_MODULE, 1));
        Serial.printf("🧪 pH Reading: ");
        if (value < 0) {
            Serial.println("Error");
        } else {
            // Converts (hundredths of pH) to pH sensor data
            Serial.printf("%.2f pH\n", CONVERT_PH(value));
            // Serial.printf("%.1f pH\n", value / 100.0);  // Same as using CONVERT_PH()
        }
    } else {
        Serial.println("⚠️ EQXPH Module not detected.");
    }

    delay(LOOP_DELAY_MS);
}