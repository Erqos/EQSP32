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
 * - Use the EQConnect mobile app to set up Wi-Fi credentials and the correct timezone 
 *   for the EQSP32 module. Without this setup, the device cannot synchronize its time 
 *   with an NTP server.
 *
 * Features:
 * - Prints whether the time is synced or not.
 * - Displays all time components via the serial monitor.
 */

 #include <EQSP32.h>  // Include the EQSP32 library

 // Create an instance of the EQSP32 library
 EQSP32 eqsp32;
 
 void setup() {
     // Initialize serial communication for debugging
     Serial.begin(115200);
 
     // Display a startup message
     Serial.println("\n🔄 Starting EQSP32 Full Local Time Demo...");
     Serial.println("📲 Please ensure Wi-Fi credentials and timezone are configured via the EQConnect mobile app.");
 
     // Initialize the EQSP32 module
     eqsp32.begin();
 }
 
 void loop() {
     // Check if the local time is synced with the NTP server
     if (!eqsp32.isLocalTimeSynced()) {
         Serial.println("\n❌ NTP Server NOT Synced.");
         Serial.println("⏳ Default time is being used. (Not valid for real-time applications)");
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
         Serial.printf("⏰ Time: %02d:%02d:%02d\n", hour, minutes, seconds);
     }
 
     // Wait 1 second before refreshing the time data
     delay(1000);
 }
 