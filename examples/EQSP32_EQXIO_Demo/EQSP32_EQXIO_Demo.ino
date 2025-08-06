 /**
 * @file EQSP32_EQXIO_Demo.ino
 * @brief Demonstrates how to use the EQXIO expansion module with Digital Input (DIN) and Power Output (POUT).
 *
 * This example demonstrates how to:
 * - Detect and control an EQXIO module connected to the EQSP32 system.
 * - Read a button press using a Digital Input (DIN) on the EQXIO module.
 * - Drive an LED or load using a Power Output (POUT) with PWM.
 * - Show automatic mode switching from POUT to DIN when output is set to 0.
 *
 * Requirements:
 * - An EQXIO expansion module must be connected to the EQSP32 expansion port.
 * - The module must be detected during the initial call to eqsp32.begin().
 *
 * Detection Behavior:
 * - Modules are auto-detected only once during the EQSP32 begin() sequence.
 * - If a module is not present during begin(), it will not be recognized later.
 *
 * Hardware Setup:
 * - Button connected to EQXIO Pin 1 (used as DIN).
 * - LED or other PWM-capable load connected to EQXIO Pin 2 (used as POUT).
 *
 * Features:
 * - Reads digital input from the expansion module.
 * - Controls power output based on input state.
 * - Displays current pin mode to confirm auto-switching behavior.
 *
 * @author Erqos Technologies
 * @date 2025-08-05
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

    // If EQXIO is not detected on boot the program will stay here
    while (!eqsp32.isModuleDetected(EQXIO(EQXIO_INDEX))) {
        Serial.print("❌ EQXIO not detected. Connect the EQXIO module and reboot.\n");
        delay(1000);
    }

    Serial.println("✅ EQXIO module detected!");

    // Ensure LED/Output starts OFF
    eqsp32.pinValue(EQXIO_LED_PIN, 0);
}

void loop() {
	// Recheck module presence
    if (!eqsp32.isModuleDetected(EQXIO(EQXIO_INDEX))) {
        Serial.println("⚠️ EQXIO module disconnected!");
        delay(LOOP_DELAY_MS);
        return;
    }

	// Read Digital Input (Button)
	bool buttonState = eqsp32.readPin(EQXIO_BUTTON_PIN) > 0;

	// Toggle Output: If button is pressed, turn ON; otherwise, turn OFF
	int outputValue = buttonState ? 1000 : 0;
	eqsp32.pinValue(EQXIO_LED_PIN, outputValue);

	// Read and Display the Current Mode of EQXIO_LED_PIN
	EQ_PinMode currentMode = eqsp32.readMode(EQXIO_LED_PIN);

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