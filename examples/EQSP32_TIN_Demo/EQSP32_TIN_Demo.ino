/**
 * @file EQSP32_TIN_demo.ino
 * @brief Demonstrates the configuration and reading of a Temperature Input (TIN) pin using the EQSP32 library.
 *
 * This example configures Pin 1 on the EQSP32 module as a Temperature Input (TIN) mode. 
 * It uses an NTC thermistor (e.g., NTCLE413E2103F106A) connected to the 5V Vout of the EQSP32 for temperature measurement. 
 * The pin is configured with specific thermistor parameters such as beta value and reference resistance. 
 * The program continuously reads and displays the temperature or detects open/short circuit conditions.
 *
 * Hardware Setup:
 * - Connect an NTC thermistor between Pin 1 and the 5V Vout pin of the EQSP32.
 * - Ensure proper wiring to avoid short circuits and verify the 5V output reference.
 *
 * Features:
 * - Temperature measurements in Celsius.
 * - Detection of sensor open or short circuit conditions.
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

// Pin definitions
#define TEMPERATURE_SENSOR_PIN 1  // Pin configured for temperature input

// Thermistor parameters (in this example NTCLE413E2103F106A was used)
#define NTC_BETA            3435    // Beta value of the thermistor
#define NTC_REF_RESISTANCE  10000   // Reference resistance in Ohms

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    // Display a startup message
    Serial.println("\nStarting EQSP32 TIN Demo...");

    // Initialize the EQSP32 module
    eqsp32.begin(); // Verbose mode disabled, default configs

    // Configure the temperature input pin
    eqsp32.pinMode(TEMPERATURE_SENSOR_PIN, TIN);  
    eqsp32.configTIN(TEMPERATURE_SENSOR_PIN, NTC_BETA, NTC_REF_RESISTANCE);
}

void loop() {
    // Read the temperature or check for errors
    Serial.print("Reading TIN: ");
    int tempValue = eqsp32.readPin(TEMPERATURE_SENSOR_PIN);

    if (tempValue == TIN_OPEN_CIRCUIT) {
        Serial.println("OPEN CIRCUIT detected!");
    } else if (tempValue == TIN_SHORT_CIRCUIT) {
        Serial.println("SHORT CIRCUIT detected!");
    } else {
        Serial.print(tempValue / 10.0); // Divide by 10 to convert to Celsius
        Serial.println(" °C");
    }

    // Add a small delay for stability
    delay(2000);
}
