/**
 * @file EQSP32_WebServer_Basic_UI_Demo.ino
 * @brief Minimal EQSP32 Web UI to control a single relay with real-time clock display.
 *
 * This example demonstrates how to:
 * - Host a simple web interface from the EQSP32 device.
 * - Control a relay output (fan) using a toggle button on the web page.
 * - Display the current relay state and synchronized local time.
 * 
 * Connectivity Notes:
 * - Wi-Fi credentials and optional static IP (for either Wi-Fi or Ethernet) are configured via the EQConnect mobile app.
 * - By default, EQSP32 uses DHCP unless changed via EQConnect.
 * - The web server is accessible via:
 *    - The IP address, DHCP/static (e.g., http://192.168.1.42)
 *    - or the device name, which might not be available in case of static IP (e.g., http://My-Device.home)
 *
 * Requirements:
 * - EQSP32 must be connected to the internet via Wi-Fi or Ethernet.
 * - Network credentials (for Wi-Fi) must be set using the EQConnect mobile app.
 * - Timezone must also be configured via EQConnect for accurate local time display.
 *
 * Hardware Setup:
 * - One relay output connected to `EQ_PIN_12` to control a fan or load.
 * - No physical input buttons used; all control is virtual via web interface.
 *
 * Features:
 * - Simple and responsive HTML UI for relay control.
 * - Displays local time synced via NTP.
 * - Automatically starts the web server once the device is online and time is synchronized.
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */


#include <EQSP32.h>
#include <WiFi.h>
#include <WebServer.h>


// ========== EQSP32 & SERVER SETUP ==========
EQSP32 eqsp32;
WebServer server(80);
bool serverStarted = false;


// ========== PIN DEFINITIONS ==========
#define PIN_RELAY_FAN   EQ_PIN_12


// ================== OTHER SYSTEM CONSTANTS ==================
#define RELAY_HOLD_POWER        600         // Holding PWM value set at 60%
#define RELAY_DERATE_TIME       1500        // Derate time in milliseconds


void handleRootPage();
void toggleFan();


// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    Serial.println("\n🚀 Starting EQSP32 Webserver Basic UI Demo ...");

    eqsp32.begin();

    // Configure relay pin
    eqsp32.pinMode(PIN_RELAY_FAN, RELAY);
    eqsp32.configRELAY(PIN_RELAY_FAN, RELAY_HOLD_POWER, RELAY_DERATE_TIME);
    eqsp32.pinValue(PIN_RELAY_FAN, 0);  // Initially OFF

    // Web routes
    server.on("/", handleRootPage);
    server.on("/toggleFan", toggleFan);
}


// ========== MAIN LOOP ==========
void loop() {
    // -- Start the web server once device is online and ready
    if (!serverStarted && eqsp32.isDeviceOnline() && eqsp32.isLocalTimeSynced()) {
        Serial.println("🌐 Device Online and Ready — Starting WebServer ...");
        server.begin();

        Serial.print("📡 Access EQSP32 at: http://");
        Serial.println(eqsp32.localIP());
		Serial.print("and maintenance panel at: http://");
        Serial.print(eqsp32.localIP());
		Serial.println(":8000\n");
        serverStarted = true;
    }

    if (serverStarted) server.handleClient();

    delay(100);  // FreeRTOS friendly
}


// ========== ROOT PAGE HANDLER ==========
void handleRootPage() {
    String html = "<html><body><h2>EQSP32 Basic UI</h2>";
    html += "<p>Fan State: " + String(eqsp32.readPin(PIN_RELAY_FAN) != 0 ? "On" : "Off") + "</p>";

    int hour = eqsp32.getLocalHour();
    int min = eqsp32.getLocalMins();
    int sec = eqsp32.getLocalSecs();

    html += "<p>Local Time: ";
    if (hour < 10) html += "0";
    html += String(hour) + ":";
    if (min < 10) html += "0";
    html += String(min) + ":";
    if (sec < 10) html += "0";
    html += String(sec) + "</p>";

    html += "<form action=\"/toggleFan\"><button>Toggle Fan</button></form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}


// ========== HANDLER FUNCTION ==========
void toggleFan() {
    int state = eqsp32.readPin(PIN_RELAY_FAN);
    int newValue = state ? 0 : 1000;
    eqsp32.pinValue(PIN_RELAY_FAN, newValue);
    server.sendHeader("Location", "/");
    Serial.println(String("🌀 : ") + (newValue ? "✅ ON" : "❌ OFF") + " - Fan");
    server.send(303);
}