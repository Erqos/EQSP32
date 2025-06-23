/**
 * @file EQSP32_WebServer_Basic_UI_Demo.ino
 * @brief Minimal web interface to control a single relay using EQSP32.
 *
 * This example demonstrates:
 * - Static IP configuration and EQConnect provisioning with EQSP32.
 * - Virtual-only control of one relay output (no physical inputs).
 * - A simple HTML interface with a toggle button and status display.
 * - Real-time local time display via EQSP32 NTP synchronization.
 *
 * Hardware Setup:
 * - One RELAY output on EQ_PIN_12 driving a load (e.g., fan).
 * - No manual switches or inputs used.
 *
 * @date 2025-06-17
 * @author Erqos Technologies
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


// ========== SETUP ==========
void setup() {
    Serial.begin(115200);

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
    // -- Start the web server once WiFi is connected
    if (!serverStarted && eqsp32.getWiFiStatus() == EQ_WF_CONNECTED && eqsp32.isLocalTimeSynced()) {
        Serial.println("🌐 WiFi Connected — Starting WebServer...");
        server.begin();

        // Optional: Set static IP (you may remove this section)
        IPAddress local_IP(192, 168, 1, 42);
        IPAddress gateway(192, 168, 1, 1);
        IPAddress subnet(255, 255, 255, 0);
        if (!WiFi.config(local_IP, gateway, subnet)) {
            Serial.println("❌ Failed to configure static IP");
        }

        Serial.print("📡 Access EQSP32 at: http://");
        Serial.println(WiFi.localIP());  // Consider replacing with mDNS in the future
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
