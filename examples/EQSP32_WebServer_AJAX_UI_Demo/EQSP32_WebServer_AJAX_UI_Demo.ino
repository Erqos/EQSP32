/**
 * @file EQSP32_WebServer_AJAX_UI_Demo.ino
 * @brief Interactive web interface with real-time AJAX updates and local time display using EQSP32.
 *
 * This example demonstrates:
 * - Initialization of EQSP32 with static IP and EQConnect provisioning enabled.
 * - Ambient temperature monitoring via NTC thermistor with smoothing.
 * - Manual switch control for two lights (ceiling and hidden) via hardware buttons.
 * - Virtual control of fan and ventilation via a responsive web interface (AJAX).
 * - Real-time status updates using a JSON API (temperature, relay states, switch states).
 * - Real-time display of local time using EQSP32 time functions.
 *
 * - Responsive HTML UI with dynamic feedback and color-coded temperature gauge.
 * - Modular HTML and JavaScript generation for easier maintenance.
 * - EQTimer used for periodic IP display in serial output.
 *
 * Hardware Setup:
 * - EQSP32 board powered by an external 24V DC supply.
 * - NTC thermistor (e.g., NTCLE413E2103F106A) connected between terminal 1 (TIN) and EQSP32’s 5V output.
 * - Toggle switches (SPST) wired between EQSP32 5V supply (or VIn) and SWT inputs (EQ_PIN_3, EQ_PIN_4).
 * - Four relay outputs for load control (EQ_PIN_12, 13, 14, 16) switching on the low side.
 *
 * @date 2025-06-17
 * @author Erqos Technologies
 */

#include <EQSP32.h>
#include <WiFi.h>
#include <WebServer.h>


// ================== EQSP32 & SERVER SETUP ==================
EQSP32 eqsp32;
WebServer server(80);
bool serverStarted = false;

EQTimer printIP(30000);  // Timer to periodically print IP


// ================== PIN DEFINITIONS ==================
// Inputs
#define PIN_AMBIENT_TEMP        EQ_PIN_1    // Ambient temperature sensor (TIN)
#define PIN_SWITCH_1            EQ_PIN_3    // Manual switch for ceiling light (SWT)
#define PIN_SWITCH_2            EQ_PIN_4    // Manual switch for hidden light (SWT)

// Outputs
#define PIN_RELAY_FAN           EQ_PIN_12   // Fan controlled via web UI (RELAY)
#define PIN_RELAY_VENT          EQ_PIN_13   // Ventilation controlled via web UI (RELAY)
#define PIN_RELAY_CEILING       EQ_PIN_14   // Ceiling light controlled by manual button (RELAY)
#define PIN_RELAY_HIDDEN_LIGHT  EQ_PIN_16  // Hidden light controlled by manual button (RELAY)


// ================== TEMPERATURE RANGE CONSTANTS ==================
#define TEMP_MIN            10
#define TEMP_MAX            40
#define COLD_LIMIT          18
#define HOT_LIMIT           25


// ================== OTHER SYSTEM CONSTANTS ==================
#define RELAY_HOLD_POWER        600         // Holding PWM value set at 60%
#define RELAY_DERATE_TIME       1500        // Derate time in milliseconds

#define NTC_BETA_VALUE          3435        // Beta value of the NTC thermistor (NTCLE413E2103F106A)
#define NTC_REF_RESISTANCE      10000       // Resistance at 25°C (e.g. for NTCLE413E2103F106A is 10kΩ)

#define SMOOTH_TEMP_SAMPLES 10              // Number of samples to average for temperature


// ================== SYSTEM SETUP ==================
void setup() {
    Serial.begin(115200);

    // -- Initialize EQSP32 (uses EQConnect for credentials)
    eqsp32.begin();

    // -- Configure inputs
    eqsp32.pinMode(PIN_SWITCH_1, SWT);  // Manual switch for ceiling light
    eqsp32.pinMode(PIN_SWITCH_2, SWT);  // Manual switch for hidden light
    eqsp32.pinMode(PIN_AMBIENT_TEMP, TIN);  // Ambient temperature sensor
    eqsp32.configTIN(PIN_AMBIENT_TEMP, NTC_BETA_VALUE, NTC_REF_RESISTANCE);  // Configure ambient temp sensor

    // -- Configure outputs
    eqsp32.pinMode(PIN_RELAY_CEILING, RELAY);
    eqsp32.configRELAY(PIN_RELAY_CEILING, RELAY_HOLD_POWER, RELAY_DERATE_TIME);
    eqsp32.pinValue(PIN_RELAY_CEILING, 0);

    eqsp32.pinMode(PIN_RELAY_HIDDEN_LIGHT, RELAY);
    eqsp32.configRELAY(PIN_RELAY_HIDDEN_LIGHT, RELAY_HOLD_POWER, RELAY_DERATE_TIME);
    eqsp32.pinValue(PIN_RELAY_HIDDEN_LIGHT, 0);

    eqsp32.pinMode(PIN_RELAY_FAN, RELAY);
    eqsp32.configRELAY(PIN_RELAY_FAN, RELAY_HOLD_POWER, RELAY_DERATE_TIME);
    eqsp32.pinValue(PIN_RELAY_FAN, 0);

    eqsp32.pinMode(PIN_RELAY_VENT, RELAY);
    eqsp32.configRELAY(PIN_RELAY_VENT, RELAY_HOLD_POWER, RELAY_DERATE_TIME);
    eqsp32.pinValue(PIN_RELAY_VENT, 0);


    // -- Web server route handling
    server.on("/", handleRoot);
    server.on("/handleToggleFan", handleToggleFan);
    server.on("/handleToggleVent", handleToggleVent);
    server.on("/status", handleStatus);

    printIP.start();
}


// ================== MAIN LOOP ==================
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

    // -- Print IP periodically for monitoring
    if (printIP.isExpired()) {
        Serial.print("📡 Access EQSP32 at: http://");
        Serial.println(WiFi.localIP());
        printIP.reset();
    }

    // -- Handle web requests
    if (serverStarted) {
        server.handleClient();
    }

    // -- Update manual button states (lights)
    handleManualButtons();

    delay(100);  // 100 ms loop delay for FreeRTOS timing
}



// ================== WEB CONTROL HANDLERS ==================
// Toggles the fan relay when called from the web interface
void handleToggleFan() {
    int state = eqsp32.readPin(PIN_RELAY_FAN);
    int newState = state ? 0 : 1000;
    eqsp32.pinValue(PIN_RELAY_FAN, newState);
    Serial.println(String("🌀 : ") + (newState ? "✅ ON" : "❌ OFF") + " - Fan");
    server.send(200, "text/plain", "OK");
}

// Toggles the ventilation relay when called from the web interface
void handleToggleVent() {
    int state = eqsp32.readPin(PIN_RELAY_VENT);
    int newState = state ? 0 : 1000;
    eqsp32.pinValue(PIN_RELAY_VENT, newState);
    Serial.println(String("💨 : ") + (newState ? "✅ ON" : "❌ OFF") + " - Ventilation");
    server.send(200, "text/plain", "OK");
}


// ================== MANUAL CONTROL HANDLER ==================
// Reads the state of the physical switches and updates the respective outputs
void handleManualButtons() {
    // Ceiling Light is controlled by Switch 1
    eqsp32.pinValue(PIN_RELAY_CEILING, eqsp32.readPin(PIN_SWITCH_1) == 0 ? 0 : 1000);

    // Hidden Light is controlled by Switch 2
    eqsp32.pinValue(PIN_RELAY_HIDDEN_LIGHT, eqsp32.readPin(PIN_SWITCH_2) == 0 ? 0 : 1000);
}


// ================== STATUS ENDPOINT ==================
// Sends a JSON response with the current state of sensors and outputs
void handleStatus() {
    String json = "{";

    json += "\"temp\":" + String(getSmoothedTemperature(), 1);                      // Ambient temperature
    json += ",\"fan\":" + String(eqsp32.readPin(PIN_RELAY_FAN));                    // Virtual Fan control
    json += ",\"ventilation\":" + String(eqsp32.readPin(PIN_RELAY_VENT));           // Virtual Ventilation control
    json += ",\"ceiling\":" + String(eqsp32.readPin(PIN_RELAY_CEILING));            // Manual Ceiling Light
    json += ",\"hidden\":" + String(eqsp32.readPin(PIN_RELAY_HIDDEN_LIGHT));        // Manual Hidden Light
    json += ",\"switch1\":" + String(eqsp32.readPin(PIN_SWITCH_1));                 // Switch 1 state
    json += ",\"switch2\":" + String(eqsp32.readPin(PIN_SWITCH_2));                 // Switch 2 state

    json += ",\"hour\":" + String(eqsp32.getLocalHour());
    json += ",\"min\":" + String(eqsp32.getLocalMins());
    json += ",\"sec\":" + String(eqsp32.getLocalSecs());

    json += "}";
    server.send(200, "application/json", json);

    // Serial.println("Status update called.");     // Uncomment this to debug if periodic update function is called
}


// ================== ROOT PAGE HANDLER ==================
// Serves the main HTML UI by combining modular sections and scripts
void handleRoot() {
    float temp = getSmoothedTemperature();

    int sw1 = eqsp32.readPin(PIN_SWITCH_1);
    int sw2 = eqsp32.readPin(PIN_SWITCH_2);

    int relay1 = eqsp32.readPin(PIN_RELAY_FAN);
    int relay2 = eqsp32.readPin(PIN_RELAY_VENT);
    int relay3 = eqsp32.readPin(PIN_RELAY_CEILING);
    int relay4 = eqsp32.readPin(PIN_RELAY_HIDDEN_LIGHT);

    String html = buildPageHeader();
    html += buildSectionTemperature(temp);
    html += buildSectionTime();
    html += buildSectionControls();
    html += buildSectionInputs(sw1, sw2);
    html += buildSectionOutputs(relay1, relay2, relay3, relay4);
    html += buildScriptConstants();
    html += buildScriptFunctions();
    html += "</body></html>";

    server.send(200, "text/html; charset=utf-8", html);
}



// ================== HELPER FUNCTIONS ==================
// -- getSmoothedTemperature: Returns the moving average of the ambient temperature
float getSmoothedTemperature() {
    static float samples[SMOOTH_TEMP_SAMPLES] = {0};  // Sample buffer
    static int index = 0;                             // Current index in buffer
    static int count = 0;                             // Number of samples added

    float newReading = eqsp32.readPin(PIN_AMBIENT_TEMP) / 10.0;  // Convert to °C
    samples[index] = newReading;
    index = (index + 1) % SMOOTH_TEMP_SAMPLES;
    if (count < SMOOTH_TEMP_SAMPLES) count++;

    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += samples[i];
    }

    return sum / count;  // Return average
}


// ================== HTML HELPER FUNCTIONS ==================
//
// These modular functions generate parts of the web interface used in handleRoot().
// Keeping them separate allows easier maintenance and readability.
// ===========================================================
// -- buildPageHeader: Returns the HTML header, styles, and page title
String buildPageHeader() {
    String html;
    html += "<html><head><title>EQSP32 Workshop Control</title>";
    html += "<link rel='icon' type='image/png' href='https://erqos.com/wp-content/uploads/2023/11/cropped-cropped-eq-logo-white-space.png'>";
    html += "<style>";
    html += "body { font-family: 'Segoe UI', sans-serif; background: #eef2f7; margin: 0; padding: 0; }";
    html += "h2 { background: #007bff; color: white; padding: 20px 0; margin: 0; text-align: center; }";
    html += "section { background: white; margin: 20px auto; padding: 20px; width: 90%; max-width: 800px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); border-radius: 10px; text-align: center; }";
    html += ".gauge { position: relative; height: 25px; background: #ddd; border-radius: 20px; overflow: hidden; }";
    html += ".gauge-segment { position: absolute; top: 0; height: 100%; }";
    html += "#gauge-blue { background: #a0cfff; }";
    html += "#gauge-green { background: #b3f0b3; }";
    html += "#gauge-red { background: #ff9999; }";
    html += "#needle { position: absolute; top: -6px; width: 2px; height: 37px; background: #f2f2f2; }";
    html += ".btn-group { display: flex; justify-content: center; gap: 20px; flex-wrap: wrap; }";
    html += ".button { padding: 12px 24px; font-size: 16px; background: #aaa; color: white; border: none; border-radius: 8px; text-decoration: none; transition: 0.3s; min-width: 160px; }";
    html += ".button.active { background: #218721; }";
    html += ".led { display: inline-block; width: 20px; height: 20px; margin: 0 10px; border-radius: 50%; }";
    html += ".switch-led.off { background: #114000; } .switch-led.on { background: #218721; }";
    html += ".relay-led.off { background: #5d1100; } .relay-led.on { background: #d0011b; }";
    html += "p { font-size: 18px; margin: 5px 0; }";
    html += "</style></head><body>";
    html += "<h2>EQSP32 Workshop Control</h2>";
    return html;
}

// -- buildSectionTemperature: Builds the ambient temperature gauge section
String buildSectionTemperature(float temp) {
    String html;
    html += "<section><h3>Ambient Temperature</h3>";
    html += "<div class='gauge'>";
    html += "<div id='gauge-blue' class='gauge-segment'></div>";
    html += "<div id='gauge-green' class='gauge-segment'></div>";
    html += "<div id='gauge-red' class='gauge-segment'></div>";
    html += "<div id='needle'></div>";
    html += "</div>";
    html += "<p><span id='tempValue'>" + String(temp, 1) + "</span> &deg;C</p></section>";
    return html;
}

// -- buildSectionControls: Generates HTML for virtual fan/ventilation buttons
String buildSectionControls() {
    String html;
    html += "<section><h3>Virtual Controls</h3><div class='btn-group'>";
    html += "<button id='btnFan' class='button' onclick='toggleFan()'>Toggle Fan</button>";
    html += "<button id='btnVentilation' class='button' onclick='toggleVentilation()'>Toggle Ventilation</button>";
    html += "</div></section>";
    return html;
}

// -- buildSectionInputs: Displays state of physical switches
String buildSectionInputs(int sw1, int sw2) {
    String html;
    html += "<section><h3>Manual Switches</h3><div class='btn-group'>";
    html += "<div><div id='switch1' class='led switch-led " + String(sw1 ? "on" : "off") + "'></div> Ceiling Light Switch</div>";
    html += "<div><div id='switch2' class='led switch-led " + String(sw2 ? "on" : "off") + "'></div> Hidden Light Switch</div>";
    html += "</div></section>";
    return html;
}

// -- buildSectionOutputs: Shows the state of each relay
String buildSectionOutputs(int fan, int vent, int ceiling, int hidden) {
    String html;
    html += "<section><h3>Output States</h3><div class='btn-group'>";
    html += "<div><div id='relay1' class='led relay-led " + String(fan ? "on" : "off") + "'></div> Fan</div>";
    html += "<div><div id='relay2' class='led relay-led " + String(vent ? "on" : "off") + "'></div> Ventilation</div>";
    html += "<div><div id='relay3' class='led relay-led " + String(ceiling ? "on" : "off") + "'></div> Ceiling Light</div>";
    html += "<div><div id='relay4' class='led relay-led " + String(hidden ? "on" : "off") + "'></div> Hidden Light</div>";
    html += "</div></section>";
    return html;
}

// -- buildSectionTime: Shows the local EQSP32 time from NTP server sync
String buildSectionTime() {
    String html;
    html += "<section><h3>Local Time</h3>";
    html += "<p id='localTime'>--:--:--</p>";
    html += "</section>";
    return html;
}

// -- buildScriptConstants: Injects system limits/constants into JS
String buildScriptConstants() {
    String html;
    html += "<script>";
    html += "const TEMP_MIN = " + String(TEMP_MIN) + ";";
    html += "const TEMP_MAX = " + String(TEMP_MAX) + ";";
    html += "const COLD_LIMIT = " + String(COLD_LIMIT) + ";";
    html += "const HOT_LIMIT = " + String(HOT_LIMIT) + ";";
    html += "</script>";
    return html;
}

// -- buildScriptFunctions: Adds JS for interactivity and real-time updates
String buildScriptFunctions() {
    return R"rawliteral(
    <script>
    function toggleFan() {
        fetch('/handleToggleFan').then(() => updateStatus());
    }
    function toggleVentilation() {
        fetch('/handleToggleVent').then(() => updateStatus());
    }
    function updateStatus() {
        fetch('/status')
        .then(res => res.json())
        .then(data => {
            document.getElementById('tempValue').innerText = parseFloat(data.temp).toFixed(1);
            let temp = data.temp;

            let percent = Math.min(Math.max((temp - TEMP_MIN) * 100 / (TEMP_MAX - TEMP_MIN), 0), 100);

            const blueEnd = (COLD_LIMIT - TEMP_MIN) * 100 / (TEMP_MAX - TEMP_MIN);
            const greenEnd = (HOT_LIMIT - TEMP_MIN) * 100 / (TEMP_MAX - TEMP_MIN);

            let blueWidth = 0, greenWidth = 0, redWidth = 0;
            if (percent <= blueEnd) {
                blueWidth = percent;
            } else if (percent <= greenEnd) {
                blueWidth = blueEnd;
                greenWidth = percent - blueEnd;
            } else {
                blueWidth = blueEnd;
                greenWidth = greenEnd - blueEnd;
                redWidth = percent - greenEnd;
            }

            document.getElementById("gauge-blue").style.width = blueWidth + "%";
            document.getElementById("gauge-green").style.left = blueWidth + "%";
            document.getElementById("gauge-green").style.width = greenWidth + "%";
            document.getElementById("gauge-red").style.left = (blueWidth + greenWidth) + "%";
            document.getElementById("gauge-red").style.width = redWidth + "%";
            document.getElementById("needle").style.left = percent + "%";

            const update = (id, state, cls) => {
                const el = document.getElementById(id);
                el.className = "led " + cls + " " + (state ? "on" : "off");
            };

            update("relay1", data.fan, "relay-led");
            update("relay2", data.ventilation, "relay-led");
            update("relay3", data.ceiling, "relay-led");
            update("relay4", data.hidden, "relay-led");

            update("switch1", data.switch1, "switch-led");
            update("switch2", data.switch2, "switch-led");

            document.getElementById("btnFan").className = "button" + (data.fan ? " active" : "");
            document.getElementById("btnVentilation").className = "button" + (data.ventilation ? " active" : "");

            // === Local Time Display ===
            let h = String(data.hour).padStart(2, '0');
            let m = String(data.min).padStart(2, '0');
            let s = String(data.sec).padStart(2, '0');
            document.getElementById('localTime').innerText = `${h}:${m}:${s}`;
        });
    }
    setInterval(updateStatus, 250);
    </script>
    )rawliteral";
}

