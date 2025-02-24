/**
 * @file EQSP32_EQXIO_Demo.ino
 * @brief Demonstrates the usage of an EQXIO expansion module with Digital Input (DIN) and Power Output (POUT).
 *
 * This example connects an EQXIO module to the EQSP32 and configures:
 * - A digital input (DIN) to read a button press.
 * - A power output (POUT) to control an LED or external load.
 * - Detects automatic switching between POUT → DIN when the output is set to 0.
 *
 * Hardware Setup:
 * - EQXIO Module connected to EQSP32 (first EQXIO module is indexed as 1).
 * - Button connected to EQXIO Pin 1 (Digital Input).
 * - LED or Load connected to EQXIO Pin 2 (PWM Output).
 *
 * Features:
 * - Reads a button press from the expansion module.
 * - Controls an LED/motor using PWM.
 * - Monitors the automatic mode switching on the POUT pin.
 *
 * @author Erqos Technologies
 * @date 2025-02-20
 */

#include <EQSP32.h>  // Include the EQSP32 library

EQSP32 eqsp32;

// Define EQXIO module index (first module starts from 1)
#define EQXIO_INDEX  1  

// EQXIO Pin Assignments
#define EQXIO_BUTTON_PIN  EQXIO(EQXIO_INDEX, EQXIO_PIN_1)  // Digital Input (DIN)
#define EQXIO_LED_PIN     EQXIO(EQXIO_INDEX, EQXIO_PIN_2)  // Power Output (POUT)

// Loop Delay
#define LOOP_DELAY_MS 500

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\nStarting EQSP32 EQXIO Demo...");

    // Initialize EQSP32
    eqsp32.begin();  // Default initialization

    // Configure EQXIO Pins
    eqsp32.pinMode(EQXIO_BUTTON_PIN, DIN);  // Configure as Digital Input
    eqsp32.pinMode(EQXIO_LED_PIN, POUT);    // Configure as Power Output (PWM)

    // Ensure LED/Output starts OFF
    eqsp32.pinValue(EQXIO_LED_PIN, 0);
}

void loop() {
    // Read Digital Input (Button)
    bool buttonState = eqsp32.readPin(EQXIO_BUTTON_PIN) > 0;

    // Toggle Output: If button is pressed, turn ON; otherwise, turn OFF
    int outputValue = buttonState ? 1000 : 0;
    eqsp32.pinValue(EQXIO_LED_PIN, outputValue);

    // Read and Display the Current Mode of EQXIO_LED_PIN
    PinMode currentMode = eqsp32.readMode(EQXIO_LED_PIN);

    // Print a divider for better readability
    Serial.println("\n========================================");

    // Print Debugging Information
    Serial.print("🔘 Button State: \t");
    Serial.println(buttonState ? "🟢 PRESSED" : "⚫ RELEASED");

    Serial.print("💡 Output State: \t");
    Serial.println(outputValue ? "🟡 ON (PWM)" : "⚫ OFF (DIN Mode)");

    Serial.print("📌 EQXIO_LED_PIN Mode: ");
    Serial.println(currentMode == POUT ? "\"POUT\"" : "\"DIN\"");  // Shows automatic mode switch

    // Add a small delay
    delay(LOOP_DELAY_MS);
}
