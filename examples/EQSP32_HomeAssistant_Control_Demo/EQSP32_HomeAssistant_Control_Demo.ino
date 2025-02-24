/**
 * @file EQSP32_HomeAssistant_Control_Demo.ino
 * @brief Demonstrates EQSP32 relay control using Home Assistant via MQTT.
 *
 * This example configures the EQSP32 module to:
 * - Control a relay from Home Assistant.
 * - Use EQTimer to keep the relay on for a user-defined time.
 * - Automatically turn off the relay when the timer expires.
 * - Update Home Assistant switch state when the relay turns off.
 *
 * Hardware Setup:
 * - EQSP32 connected to WiFi and MQTT broker (configured via EQConnect).
 * - Relay connected to an EQSP32 output pin.
 *
 * Features:
 * - Uses MQTT discovery for seamless integration with Home Assistant.
 * - Controls relay using a switch in Home Assistant.
 * - Uses a Home Assistant number input to define the relay ON duration.
 * - Automatically turns off relay when the time expires.
 *
 * @author Erqos Technologies
 * @date 2025-02-22
 */

#include <EQSP32.h>   // Include the EQSP32 library

void printSystemState();  // Function prototype for system state logging

EQSP32 eqsp32;
EQTimer relayTimer;  // Timer instance for relay control

// Device wiring
#define RELAY_PIN 6   // Relay output pin

// Home Assistant MQTT Entity Names
std::string haRelayName = "Relay Control";  // Home Assistant switch entity for relay control
std::string haTimerName = "Relay Timer";    // Home Assistant number entity for setting duration

// Loop Delay
#define LOOP_DELAY_MS 1000  

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQSP32 Home Assistant Control Demo...");

    // Initialize EQSP32 (uses default HA MQTT broker and settings from EQConnect)
    eqsp32.begin();

    // Configure the relay pin
    eqsp32.pinMode(RELAY_PIN, RELAY);

    // Register Home Assistant MQTT Entities
    createControl_Switch(haRelayName, "mdi:power");     // Home Assistant switch for relay control
    createControl_Value(haTimerName, 1, 60, 0, "mdi:timer-outline");  // Number entity (1s - 60s) for setting duration

    // Publish initial values to Home Assistant
    updateControl_Switch(haRelayName, false);  // Ensure the switch starts OFF
    updateControl_Value(haTimerName, 10);      // Default timer value (10s)
}

void loop() {
    // Read Home Assistant values
    static int relayDuration = 0;  // Stores the current timer duration
    bool relayState = readControl_Switch(haRelayName);  // Read the switch state from Home Assistant

    // If the timer value in Home Assistant changes, reset the timer
    if (relayDuration != readControl_Value(haTimerName)) {
        relayTimer.stop();  // Stop the timer if the duration changes (if the switch is still on timer will be restarted)
    }
    relayDuration = readControl_Value(haTimerName);  // Update the stored duration

    // Print system state for debugging
    printSystemState();  

    if (relayState) {
        // If the relay is ON, start or restart the timer
        if (!relayTimer.isRunning()) {
            relayTimer.start(relayDuration * 1000); // Convert seconds to milliseconds
            Serial.println("========================================");
            Serial.println("Event: ✅ Relay ON - Timer Started!");
            Serial.println("========================================");
        }
        eqsp32.pinValue(RELAY_PIN, 1000);  // Keep relay ON while switch is ON
    } 

    // Check if the timer is running
    if (relayTimer.isRunning()) {
        // Check if the timer expired
        if (relayTimer.isExpired()) {   // Ensure this block only runs once
            relayTimer.stop();  // Stop the timer
            eqsp32.pinValue(RELAY_PIN, 0);  // Turn off the relay
            updateControl_Switch(haRelayName, false);  // Update HA switch state to OFF
            Serial.println("========================================");
            Serial.println("Event: ⏳ Timer Expired - Relay OFF, HA Switch Updated!");
            Serial.println("========================================");
        } 
    }

    // If the switch was turned OFF manually, turn off the relay & stop the timer
    if (!relayState) {
        relayTimer.stop();
        eqsp32.pinValue(RELAY_PIN, 0);
    }

    delay(LOOP_DELAY_MS);  // Add a small delay for loop stability
}

/**
 * @brief Prints system state including switch status, relay state, and timer countdown.
 */
void printSystemState() {
    // Read Home Assistant values
    bool relayState = readControl_Switch(haRelayName);  // Read the relay switch state
    int relayDuration = readControl_Value(haTimerName); // Read the timer duration from Home Assistant

    // Determine the display symbols for switch and relay states
    std::string switchSymbol = relayState ? "🟢" : "⚪";  // Green for ON, white for OFF
    std::string relaySymbol = eqsp32.readPin(RELAY_PIN) ? "🔴" : "⚫";  // Red for ON, black for OFF

    // Calculate the remaining time if the timer is running
    int remainingTime = relayTimer.isRunning() ? relayDuration - (relayTimer.value() / 1000) : 0;  // Convert ms to seconds

    // Ensure remaining time is not negative
    remainingTime = remainingTime < 0 ? 0 : remainingTime;

    // Print structured log for better readability
    Serial.println("\n========================================");
    Serial.printf("%s Switch State: %s\n", switchSymbol.c_str(), relayState ? "ON" : "OFF");
    Serial.printf("⏳ Timer Set: %d sec\n", relayDuration);
    Serial.printf("⏱️ Remaining: %d sec\n", remainingTime);
    Serial.printf("%s Relay: %s\n", relaySymbol.c_str(), eqsp32.readPin(RELAY_PIN) ? "ON" : "OFF");
}
