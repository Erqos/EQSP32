/**
 * @file EQSP32_System_Network_Status_Demo.ino
 * @brief Demonstrates checking and displaying the EQSP32 Wi-Fi status using icons.
 *
 * This example shows how to:
 * - Retrieve and display the current network status of the EQSP32 module.
 *
 * Features:
 * - Displays the current network status in real-time.
 * - Uses a switch statement to handle and print all status cases.
 *
 * Wi-Fi Status Summary:
 * - `EQ_WF_DISCONNECTED`: Wi-Fi is not connected.
 * - `EQ_WF_CONNECTED`: Wi-Fi is connected.
 * - `EQ_WF_RECONNECTING`: Attempting to reconnect to Wi-Fi.
 * - `EQ_WF_SCANNING`: Scanning for available Wi-Fi networks.
 *
 * Important:
 * - Use the EQConnect mobile app to set up Wi-Fi credentials in your EQSP32.
 * - Without proper configuration, the EQSP32 module will not be able to connect to the network.
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    Serial.println("\nStarting EQSP32 System Network Status Demo...");
    Serial.println("Please use the EQConnect mobile app to set up Wi-Fi credentials.");
    Serial.println("   Without proper setup, the EQSP32 module cannot connect to the network.\n");

    // Initialize the EQSP32 module
    eqsp32.begin(); // Verbose mode disabled
}

void loop() {
    Serial.print("System network status: ");

    // Retrieve and display the current Wi-Fi status
    switch (eqsp32.getWiFiStatus()) {
        case EQ_WF_DISCONNECTED:
            Serial.println("❌ Disconnected");
            break;
        case EQ_WF_CONNECTED:
            Serial.println("✅ Connected");
            break;
        case EQ_WF_RECONNECTING:
            Serial.println("🔄 Reconnecting...");
            break;
        case EQ_WF_SCANNING:
            Serial.println("📡 Scanning for Networks...");
            break;
        default:
            Serial.println("⚠️ Unknown Status");
            break;
    }

    // Add a delay before checking again
    delay(1000);
}
