/**
 * @file EQSP32_Read_Local_Time_Demo.ino
 * @brief Demonstrates how to retrieve and display all time-related values from EQSP32.
 *
 * This example shows how to use the EQSP32 library to:
 * - Check if the local time is synced with an NTP server.
 * - Retrieve the current year, month, day, weekday, hour, minute, second, and day of the year.
 * - Print the full date and time in a structured format.
 * 
 * Requirements:
 * - The EQSP32 must be connected to the internet via `Wi-Fi or Ethernet`.
 * - If WiFi is used, use EQConnect to configure the network credentials.
 * - `Timezone configuration is always required`, regardless of connection type.
 * - Use the EQConnect mobile app to set the correct timezone on the device.
 * - Without internet access or a configured timezone, the device cannot synchronize its time 
 *   with an NTP server or display correct local time.
 * 
 * Features:
 * - Compatible with both Ethernet and Wi-Fi.
 * - Prints whether the time is synced or not.
 * - Displays all time components via the serial monitor.
 * - Prints device name and device ID.
 * - Displays local and UTC time strings in ISO-like format.
 * - Shows Unix timestamps for both local and UTC time.
 * 
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    // Display a startup message
    Serial.println("\n🔄 Starting EQSP32 Full Local Time Demo...");
    Serial.println("📲 Connect EQSP32 to Wi-Fi or Ethernet and set timezone via the EQConnect mobile app.");
	Serial.println();

    // Initialize the EQSP32 module
    eqsp32.begin();
}

void loop() {
	// Show device identity
	Serial.printf("Device Name: %s\n", eqsp32.getDeviceName().c_str());
	Serial.printf("Device ID:   %s\n", eqsp32.getDeviceID().c_str());
	Serial.printf("Internet: %s\n", eqsp32.isDeviceOnline() ? "Online" : "Offline");
    Serial.printf("Network: %s\n\n", eqsp32.getEthernetStatus() == EQ_ETH_CONNECTED ? "Ethernet" :
                                (eqsp32.getWiFiStatus() == EQ_WF_CONNECTED ? "Wi-Fi" : "Disconnected"));


    // Check if the local time is synced with the NTP server
    if (!eqsp32.isLocalTimeSynced()) {
        Serial.println("\n❌ NTP Server NOT Synced.");
        Serial.println("⏳ Default time is being used. (Not valid for real-time applications)\n\n");
    } else {
        Serial.println("\n✅ NTP Server is Synced!");

        // Retrieve all time-related values
        int year = eqsp32.getLocalYear();
        int month = eqsp32.getLocalMonth();
        int monthDay = eqsp32.getLocalMonthDay();
        EQ_WeekDay weekDay = eqsp32.getLocalWeekDay();
        int yearDay = eqsp32.getLocalYearDay();
        int hour = eqsp32.getLocalHour();
        int minutes = eqsp32.getLocalMins();
        int seconds = eqsp32.getLocalSecs();

        // Weekday names for readability
        const char* weekDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

        // Print full date and time information
        Serial.printf("📅 Date: %04d-%02d-%02d (Day %d of the year)\n", year, month, monthDay, yearDay);
        Serial.printf("📆 Weekday: %s (%d)\n", weekDays[weekDay], weekDay);
        Serial.printf("⏰ Time: %02d:%02d:%02d\n\n", hour, minutes, seconds);

		        // Show time in formatted local and UTC strings
        Serial.printf("📍 Local Time String: %s\n", eqsp32.getFormattedLocalTime().c_str());
        Serial.printf("🌍 UTC Time String:   %s\n", eqsp32.getFormattedUnixTimestamp().c_str());

        // Show timestamps
        Serial.printf("📊 Local Epoch: %lu\n", eqsp32.getLocalUnixTimestamp());
        Serial.printf("🕓 UTC Epoch:   %lu\n\n\n", eqsp32.getUnixTimestamp());
    }

    // Wait 1 second before refreshing the time data
    delay(1000);
}