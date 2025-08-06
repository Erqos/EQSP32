/**
 * @file EQSP32_System_Network_Status_Demo.ino
 * @brief Demonstrates checking and displaying the EQSP32 Wi-Fi, Ethernet, and online status.
 *
 * This example shows how to:
 * - Retrieve and display the current Wi-Fi and Ethernet status.
 * - Check if the device is considered online.
 *
 * Features:
 * - Compatible with both Ethernet and Wi-Fi.
 * - Displays the current network connection method and status in real-time.
 * - Uses a switch statement to print all status cases.
 *
 * Wi-Fi Status Summary:
 * - `EQ_WF_DISCONNECTED`: Wi-Fi is not connected.
 * - `EQ_WF_CONNECTED`: Wi-Fi is connected.
 * - `EQ_WF_RECONNECTING`: Attempting to reconnect to Wi-Fi.
 * - `EQ_WF_SCANNING`: Scanning for available Wi-Fi networks.
 *
 * Ethernet Status Summary:
 * - `EQ_ETH_DISCONNECTED`: Ethernet cable is unplugged.
 * - `EQ_ETH_PLUGGED_IN`: Ethernet is plugged in, but not yet online.
 * - `EQ_ETH_CONNECTED`: Ethernet is connected and has internet access.
 * - `EQ_ETH_STOPPED`: Ethernet interface has been stopped or is not detected.
 *
 * Important:
 * - Use the EQConnect mobile app to set up Wi-Fi credentials in your EQSP32.
 * - EQSP32 gives Ethernet priority if both interfaces are available.
 * - Online status returns `true` if the device is connected to the internet via any interface.
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

void setup() {
    Serial.begin(115200);

    Serial.println("\nStarting EQSP32 System Network Status Demo...");
    Serial.println("Please use the EQConnect mobile app to set up Wi-Fi credentials.");
    Serial.println("   Without proper setup, the EQSP32 controller cannot connect to the network via WiFi.\n");

    eqsp32.begin(); // Verbose mode disabled
}

void loop() {
    Serial.println("\n\t=== Network Status Check ===");

    // Wi-Fi status
    Serial.print("\t");
    switch (eqsp32.getWiFiStatus()) {
        case EQ_WF_DISCONNECTED:
            Serial.print("❌\tWi-Fi:\t\t");
            Serial.println("Disconnected");
            break;
        case EQ_WF_CONNECTED:
            Serial.print("✅\tWi-Fi:\t\t");
            Serial.println("Connected (Online)");
            break;
        case EQ_WF_RECONNECTING:
            Serial.print("🔄\tWi-Fi:\t\t");
            Serial.println("Reconnecting...");
            break;
        case EQ_WF_SCANNING:
            Serial.print("📡\tWi-Fi:\t\t");
            Serial.println("Scanning for Networks...");
            break;
        default:
            Serial.print("⚠️\tWi-Fi:\t\t");
            Serial.println("Unknown Status");
            break;
    }

    // Ethernet status
    Serial.print("\t");
    switch (eqsp32.getEthernetStatus()) {
        case EQ_ETH_DISCONNECTED:
            Serial.print("❌\tEthernet:\t");
            Serial.println("Cable Disconnected");
            break;
        case EQ_ETH_PLUGGED_IN:
            Serial.print("🔌\tEthernet:\t");
            Serial.println("Plugged In (Not Online Yet)");
            break;
        case EQ_ETH_CONNECTED:
            Serial.print("✅\tEthernet:\t");
            Serial.println("Connected (Online)");
            break;
        case EQ_ETH_STOPPED:
            Serial.print("⛔\tEthernet:\t");
            Serial.println("Stopped or Not Detected");
            break;
        default:
            Serial.print("⚠️\tEthernet:\t");
            Serial.println("Unknown Status");
            break;
    }

    // Online status
    Serial.print("\t🌐\tSystem Online:\t");
    Serial.println(eqsp32.isDeviceOnline() ? "Online" : "Offline");

    delay(1000);
}