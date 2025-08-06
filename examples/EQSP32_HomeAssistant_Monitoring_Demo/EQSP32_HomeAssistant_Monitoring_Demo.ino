/**
 * @file EQSP32_HomeAssistant_Monitoring_Demo.ino
 * @brief Demonstrates EQSP32 integration with Home Assistant via MQTT for monitoring sensors.
 *
 * This example configures the EQSP32 module to:
 * - Publish sensor data (temperature) to Home Assistant.
 * - Detect and report sensor errors using a binary sensor.
 *
 * Connectivity Requirements:
 * - EQSP32 must be connected to the internet via Wi-Fi or Ethernet:
 *   - For Wi-Fi: use the EQConnect mobile app to set Wi-Fi credentials.
 *   - For Ethernet: simply connect the cable to a DHCP-enabled network.
 * - MQTT broker credentials (username & password) must be configured via EQConnect.
 *
 * Hardware Setup:
 * - Temperature sensor (NTC) connected to an EQSP32 TIN-capable pin.
 *
 * Features:
 * - Compatible with both Ethernet and Wi-Fi.
 * - Uses MQTT discovery for seamless Home Assistant integration.
 * - Publishes real-time temperature values.
 * - Reports sensor errors (e.g., open/short circuit) via a binary sensor.
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include <EQSP32.h>  // Include the EQSP32 library

EQSP32 eqsp32;

// Device wiring
#define TEMP_PIN  1   // Temperature sensor (TIN)

// Home Assistant MQTT Entity Names
std::string haTempSensorName = "Temperature Sensor";
std::string haTempErrorName = "Temperature Sensor Error";  // Home Assistant binary sensor for error detection

// Loop Delay
#define LOOP_DELAY_MS 1000  

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQSP32 Home Assistant Monitoring Demo...");

    // Initialize EQSP32 (uses settings from EQConnect)
    eqsp32.begin();

    // Configure EQSP32 temperature sensor
    eqsp32.pinMode(TEMP_PIN, TIN);
    eqsp32.configTIN(TEMP_PIN, 3950, 10000); // NTC sensor with Beta 3950 and 10kΩ reference resistor

    // Register Home Assistant MQTT Entities
    createDisplay_Sensor(haTempSensorName, 1, "°C", "mdi:thermometer", "temperature");  // Temperature sensor
    createDisplay_BinarySensor(haTempErrorName, "mdi:alert", "problem");  // Binary sensor for sensor errors

    // Publish initial values
    updateDisplay_Sensor(haTempSensorName, 0);           
    updateDisplay_BinarySensor(haTempErrorName, false);  
}

void loop() {
    int temperature = eqsp32.readPin(TEMP_PIN);  // Read temperature
    bool sensorError = !IS_TIN_VALID(temperature);  // Check if the reading is valid

    Serial.print("Reading TIN: ");

    if (sensorError) {
        Serial.println("⚠️ Wiring issue detected!");
    } else {
        Serial.printf("🌡️ Temperature: %.1f °C\n", temperature / 10.0);
        updateDisplay_Sensor(haTempSensorName, temperature / 10.0);  // Publish temperature in °C if valid
    }

    // Update Home Assistant sensor entity
    updateDisplay_BinarySensor(haTempErrorName, sensorError);  // Report error state

    delay(LOOP_DELAY_MS);  // Add a delay for stability
}
