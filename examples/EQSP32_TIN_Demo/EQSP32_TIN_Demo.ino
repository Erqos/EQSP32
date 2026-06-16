/**
 * @file EQSP32_TIN_demo.ino
 * @brief Demonstrates the configuration and reading of a Temperature Input (TIN) pin using the EQSP32 library.
 *
 * This example configures Pin 1 on the EQSP32 module as a Temperature Input (TIN) mode.
 * It uses an NTC thermistor (e.g., NTCLE100E3103JB0) connected to the 5V Vout of the
 * EQSP32 for temperature measurement.
 *
 * The example demonstrates all three supported thermistor conversion methods.
 * Only one conversion method should be enabled at a time. In this example, the Beta
 * equation is enabled by default because it is the simplest method to configure.
 *
 * Hardware Setup:
 * - Connect an NTC thermistor between Pin 1 and the 5V Vout pin of the EQSP32.
 * - Ensure proper wiring to avoid short circuits and verify the 5V output reference.
 *
 * Supported TIN Conversion Methods:
 * - Beta equation.
 * - Standard Steinhart-Hart coefficients.
 * - Normalized Steinhart-Hart coefficients.
 *
 * Features:
 * - Temperature measurements in Celsius.
 * - Detection of sensor open circuit conditions.
 * - Detection of sensor short circuit conditions.
 * - Demonstration of all supported TIN conversion methods.
 *
 * Thermistor Used:
 * - Part number: NTCLE100E3103JB0.
 * - Resistance at 25 °C: 10 kΩ.
 * - Beta value: 3977 K.
 *
 * Thermistor Parameters:
 * - Beta value and reference resistance were obtained directly from the thermistor datasheet.
 * - Normalized Steinhart-Hart coefficients were obtained directly from the thermistor datasheet
 *   (the fourth D coefficient is not used by the EQSP32 library).
 * - Standard Steinhart-Hart coefficients were calculated from the thermistor resistance table
 *   to demonstrate the alternative configuration method supported by the library.
 *
 * @author Erqos Technologies
 * @date 2026-06-15
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

// Pin definitions
#define TEMPERATURE_SENSOR_PIN 1  // Pin configured for temperature input


// Thermistor parameters (NTCLE100E3103JB0 was used in this example)
// Values obtained directly from the datasheet
#define NTC_BETA                  3977
#define NTC_REF_RESISTANCE        10000

// Standard Steinhart-Hart coefficients
// Calculated from the datasheet resistance table
#define NTC_SH_A                  1.13813823E-03
#define NTC_SH_B                  2.32466586E-04
#define NTC_SH_C                  9.57125763E-08

// Normalized Steinhart-Hart coefficients
// Obtained directly from the datasheet (D term omitted)
#define NTC_NSH_A                 3.354016E-03
#define NTC_NSH_B                 2.569850E-04
#define NTC_NSH_C                 2.620131E-06


// Error Messages for Sensor Issues
#define MSG_OPEN_CIRCUIT  "⚠️ OPEN CIRCUIT detected! Sensor disconnected."
#define MSG_SHORT_CIRCUIT "⚠️ SHORT CIRCUIT detected! Possible wiring issue."

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    // Display a startup message
    Serial.println("\nStarting EQSP32 TIN Demo...");

    // Initialize the EQSP32 module
    eqsp32.begin(); // Verbose mode disabled, default configs

    // Configure the temperature input pin
    eqsp32.pinMode(TEMPERATURE_SENSOR_PIN, TIN);

// Select ONE thermistor conversion method
    eqsp32.configTIN(TEMPERATURE_SENSOR_PIN, NTC_BETA, NTC_REF_RESISTANCE);                           // Beta equation
    // eqsp32.configTIN(TEMPERATURE_SENSOR_PIN, NTC_SH_A, NTC_SH_B, NTC_SH_C);                        // Standard Steinhart-Hart
    // eqsp32.configTIN(TEMPERATURE_SENSOR_PIN, NTC_NSH_A, NTC_NSH_B, NTC_NSH_C, NTC_REF_RESISTANCE); // Normalized Steinhart-Hart, D term is not used by the EQSP32 library
}

void loop() {
    // Read the temperature or check for errors
    Serial.print("Reading TIN: ");
    int tempValue = eqsp32.readPin(TEMPERATURE_SENSOR_PIN);

    // Check for sensor errors
    if (tempValue == TIN_OPEN_CIRCUIT) {
        Serial.println(MSG_OPEN_CIRCUIT);
    } else if (tempValue == TIN_SHORT_CIRCUIT) {
        Serial.println(MSG_SHORT_CIRCUIT);
    } else {
        // Convert to Celsius (since tempValue is in °C * 10)
        Serial.printf("🌡️ Temperature: %.1f °C\n", tempValue / 10.0);
    }

    // Add a small delay for stability
    delay(2000);
}
