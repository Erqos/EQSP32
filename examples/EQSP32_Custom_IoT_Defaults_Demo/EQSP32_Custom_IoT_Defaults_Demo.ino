/**
 * @file EQSP32_Custom_IoT_Defaults_Demo.ino
 * @brief Demonstrates how to configure developer-defined default network and IoT settings using EQSP32Configs.
 *
 * This example shows how to:
 * - Set default Wi-Fi SSID and password (used by the internal IoT stack until overridden by EQConnect).
 * - Enable or disable MQTT discovery for Home Assistant (or other MQTT solutions)
 * - Select between DHCP or Static IP assignment using the config structure.
 *
 * IP Configuration Options:
 *  A) Leave parameters unset  -> Device uses DHCP and EQConnect provisioning (recommended).
 *  B) Set parameters to "0.0.0.0" -> Explicit DHCP request (same behavior as leaving them unset).
 *  C) Provide valid addresses  -> Use static IP (must set all: staticIP, subnet, gateway, DNS).
 *
 * Reset Behavior:
 * - Holding the boot button for 3 seconds clears stored credentials and restores defaults.
 * - If developer defaults are provided (e.g., in this sketch), they will be re-applied on reset.
 * - If no developer defaults are provided, the EQSP32 library assigns its own safe defaults.
 *
 * Notes:
 * - Wi-Fi/Ethernet status and internet reachability can be queried using:
 *     eqsp32.getWiFiStatus(), eqsp32.getEthernetStatus(), eqsp32.isDeviceOnline().
 * - Local IP can be obtained via eqsp32.localIP().
 * - All fields in EQSP32Configs are optional; EQConnect provisioning always takes priority if used.
 *
 * Requirements:
 * - EQSP32 library installed (board setup per Quickstart Guide).
 * - EQConnect app for network provisioning and timezone configuration.
 *
 * @date 2025-09-06
 * @version 1.0.1 and later
 * @author Erqos Technologies
 */


#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer periodicPrint(5000);   // timer to print status every 5 seconds

// ---- Helper functions to print readable status ----
String ethStatusString(uint8_t status) {
    switch (status) {
        case EQ_ETH_DISCONNECTED: return "ETH_DISCONNECTED";
        case EQ_ETH_CONNECTED:    return "ETH_CONNECTED";
        case EQ_ETH_PLUGGED_IN:   return "ETH_PLUGGED_IN";
        case EQ_ETH_STOPPED:      return "ETH_STOPPED";
        default:                  return "ETH_ERROR";
    }
}

String wifiStatusString(uint8_t status) {
    switch (status) {
        case EQ_WF_DISCONNECTED: return "WF_DISCONNECTED";
        case EQ_WF_CONNECTED:    return "WF_CONNECTED";
        case EQ_WF_RECONNECTING: return "WF_RECONNECTING";
        case EQ_WF_SCANNING:     return "WF_SCANNING";
        default:                 return "WF_ERROR";
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 EQSP32 Custom IoT Default Configurations Demo");

    EQSP32Configs cfg;

    // ------------------------------------------------------------------------
    // Wi-Fi defaults
    // ------------------------------------------------------------------------
    // These are "developer defaults" that will be applied at boot if no other
    // user provisioning has been made through EQConnect.
    // - Leave empty to rely only on EQConnect app for Wi-Fi provisioning.
    cfg.wifiSSID     = "Default SSID";
    cfg.wifiPassword = "Default WiFi password";

    // ------------------------------------------------------------------------
    // MQTT Discovery (Home Assistant / Node-RED / Other MQTT platforms)
    // ------------------------------------------------------------------------
    // - true  = advertise device/entities automatically to cfg.mqttBrokerIp broker (default is "homeassistant.local")
    // - false = no auto-discovery (custom topics and MQTT connection with broker)
    // - omit  = use library default (false), EQConnect app can override
    cfg.mqttBrokerIp = "homeassistant.local";
    cfg.mqttDiscovery = true;

    // ------------------------------------------------------------------------
    // DHCP vs Static IP
    // ------------------------------------------------------------------------
    // Choose one of the following approaches:
    //
    // (A) DO NOTHING:
    //     - Comment out both (B) and (C).
    //     - Device uses EQSP32 library defaults and DHCP.
    //     - Recommended when relying mainly on EQConnect.
    //
    // (B) EXPLICIT DHCP:
    //     - Assign "0.0.0.0" to all IP fields.
    //     - Same effect as (A), but forces DHCP even if values were stored.
    //
    // (C) STATIC IP:
    //     - Provide valid IP addresses for all four fields.
    //     - Required: staticIP, subnet, gateway, DNS.
    //
    // --- Example: (C) Static IP configuration ---
    cfg.staticIP = "192.168.1.100";
    cfg.subnet   = "255.255.255.0";
    cfg.gateway  = "192.168.1.254";
    cfg.DNS      = "8.8.8.8";

    // ------------------------------------------------------------------------
    // Start EQSP32
    // ------------------------------------------------------------------------
    // eqsp32.begin(cfg, true):
    //   - Uses developer defaults provided here.
    //   - true = verbose logs (helpful for testing).
    //
    // eqsp32.begin(true):
    //   - Uses only EQSP32 library defaults (no developer overrides).
    eqsp32.begin(cfg, true);
    // eqsp32.begin(true);   // <-- alternative: use library defaults only

    // Show initial IP after bring-up
    Serial.print("Local IP: ");
    Serial.println(eqsp32.localIP());

    periodicPrint.start();
}

void loop() {
    // Periodically print network status every 5 seconds
    if (periodicPrint.isExpired()) {
        auto wf = eqsp32.getWiFiStatus();      // Wi-Fi status helper
        auto eth = eqsp32.getEthernetStatus(); // Ethernet status helper
        bool online = eqsp32.isDeviceOnline(); // Internet reachability

        Serial.print("WiFi: ");   Serial.print(wifiStatusString(wf));
        Serial.print("  Eth: ");  Serial.print(ethStatusString(eth));
        Serial.print("  Online: "); Serial.print(online ? "Yes" : "No");
        Serial.print("  IP: ");  Serial.println(eqsp32.localIP());

        periodicPrint.reset();
    }

    delay(10);  // Non-blocking loop style (lets FreeRTOS tasks run)
}

