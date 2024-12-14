/**
 * @file EQSP32_Read_Local_Time_Demo.ino
 * @brief Demonstrates how to check if the EQSP32's local time is synced with an NTP server.
 *
 * This example shows how to use the EQSP32 library to check whether the local time 
 * has been successfully synchronized with an NTP server. If the time is not synced, 
 * the library will provide the default time, which is not valid for real-time applications.
 *
 * Requirements:
 * - Use the EQConnect mobile app to set up Wi-Fi credentials and the correct timezone 
 *   for the EQSP32 module. Without this setup, the device cannot connect to the network 
 *   or synchronize its time with an NTP server.
 *
 * Features:
 * - Prints whether the time is synced or not.
 * - Displays the current time (either synced or default) via serial monitor.
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    // Display a startup message
    Serial.println("\nStarting EQSP32 Local Time Demo...");
    Serial.println("Please ensure Wi-Fi credentials and timezone are configured via the EQConnect mobile app.");

    // Initialize the EQSP32 module in verbose mode
    eqsp32.begin(); // Verbose mode disabled
}

void loop() {
    // Check if local time is synced with the NTP server
    if (eqsp32.isLocalTimeSynced()) {
        Serial.println("\n✅ NTP Server is Synced!");
        Serial.print("Current Local Time: ");
        eqsp32.printLocalTime(); // Print the synced local time
    } else {
        Serial.println("\n❌ NTP Server NOT Synced.");
        Serial.print("Default Time: ");
        eqsp32.printLocalTime(); // Print the default time
        Serial.println(" (Default time is not valid for real-time use)");
    }

    // Add a small delay to reduce output frequency
    delay(2000); // Wait 2 seconds before the next check
}
