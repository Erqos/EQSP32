/**
 * @file EQSP32_DashIO_Basic_Demo.ino
 * @brief Demonstrates EQSP32 hardware I/O running alongside the DashIO ESP32 library, exposing
 *        the controller in the free Dash IoT phone app over BLE, TCP, and MQTT.
 *
 * This example assumes:
 *   - Erqos IoT functionality is DISABLED (`disableErqosIoT = true`).
 *   - Wi-Fi, BLE, and MQTT are managed by the DashIO library, not by EQSP32.
 *   - The DashIO libraries are installed through the Arduino IDE Library Manager.
 *   - The dashboard layout is embedded below, so the Dash app draws the controls itself on
 *     first connect.
 *
 * Before Compiling:
 *   - Use the standard EQSP32 board settings from the Erqos quickstart guide
 *     (https://erqos.com/resources/quickstart-guide/): ESP32-S3 board, ESP32 core v2,
 *     Flash Size 8MB, Partition Scheme "8M with spiffs (3MB APP/1.5MB SPIFFS)".
 *   - Install "DashioESP" through the Library Manager. It pulls in "Dashio" and "MQTT".
 *   - Install NimBLE-Arduino 2.5.1, and keep exactly one copy. DashioESP pulls NimBLE in, so the
 *     sketch compiles its own copy alongside the one inside the EQSP32 precompiled archive. The
 *     two versions must match or the link fails with pages of "multiple definition" errors.
 *     EQSP32 v1.5.0 is built against 2.5.1. Using EQSP32 on its own needs no NimBLE install.
 *   - The Library Manager also installs a standalone "Preferences" library as a declared
 *     DashioESP dependency. It is unnecessary on ESP32 but harmless: the Arduino resolver
 *     prefers the core's own copy over the standalone one, which declares "architectures=all".
 *     Leave it installed unless a build actually names it.
 *
 * Validated Library Versions:
 *   - DashioESP        1.2.18   ESP32 transport: BLE, TCP, MQTT, and Wi-Fi (DashioESP.h).
 *   - Dashio           1.2.4    Core Dash protocol and message builders, pulled in by DashioESP.
 *   - MQTT             2.5.2    arduino-mqtt by Joel Gahwiler, used by DashMQTT.
 *   - NimBLE-Arduino   2.5.1    BLE stack required by DashioESP. MUST match the version EQSP32
 *                               is built against; see "Before Compiling".
 *   - EQSP32           1.5.0    Erqos EQSP32 library.
 *   - Arduino-ESP32    2.0.17   ESP32 core v2.
 *
 *   Install DashioESP 1.2.18 or newer. Releases 1.2.3 onward target the NimBLE 2.x API and add
 *   the BLE reconnect and message-corruption fixes. Older releases target NimBLE 1.x and are
 *   not compatible with the NimBLE version this example is built against.
 *
 * Why Erqos IoT Is Disabled:
 *   - EQSP32 and DashIO are both complete IoT stacks, and each expects to own the device's
 *     BLE, Wi-Fi, and MQTT connectivity. Only one of them can.
 *   - BLE: EQSP32 uses BLE for EQConnect provisioning, and DashIO uses BLE to talk to the Dash
 *     app. Left enabled, the two compete for the radio and neither stays reliably discoverable.
 *   - Wi-Fi: EQSP32 manages the connection, including reconnect behavior and Ethernet failover.
 *     DashIO runs its own connection and reconnect logic. The two work against each other.
 *   - MQTT: both can maintain their own broker connection. They do not conflict, but two
 *     simultaneous TLS sessions are costly in RAM on a single ESP32.
 *   - Setting `disableErqosIoT = true` hands connectivity to DashIO and resolves all three.
 *
 * Retained EQSP32 Functionality:
 *   - All ADIO terminals and pin modes, including outputs.
 *   - EQX expansion modules, RS232/RS485, and CAN.
 *   - Supply voltage sensing, buzzer, and user button.
 *   - Status LEDs, which become user-controlled and are used here as link indicators.
 *
 * Resolved Library Conflicts:
 *   - The runtime conflict above is resolved in code, by the single `disableErqosIoT` flag.
 *   - The build conflicts are resolved by the installed libraries, not by this sketch. See
 *     "Before Compiling" for the Preferences and NimBLE steps.
 *   - `DeviceData` brace initialization: the DashIO examples use aggregate initialization, which
 *     the Arduino IDE accepts at gnu++17. The fields are assigned explicitly in `setup()` here
 *     so the same sketch also compiles under PlatformIO, which builds the ESP32 core at
 *     gnu++11 and rejects it.
 *
 * Dashboard Controls:
 *   - Button `B01` enables the output on `PIN_POWER_OUT`.
 *   - Slider `SL01` sets output power, 0-100% mapped to `pinValue()` 0-1000.
 *   - Text `T01` / `T02` show the DIN and SWT input states.
 *   - Dial `D01` shows the AIN input in mV.
 *   - Text `T03` / `T04` show input and output supply voltages.
 *   - Text `T05` shows uptime.
 *
 * Hardware Setup:
 *   - One EQSP32 controller. No expansion modules or external wiring are required.
 *   - Optional input signals on `PIN_DIGITAL_IN`, `PIN_SWITCH_IN`, and `PIN_ANALOG_IN`.
 *   - Outputs remain idle unless `DEMO_DRIVE_OUTPUT` is set to 1. Confirm what is wired to
 *     `PIN_POWER_OUT` before enabling it.
 *
 * Requirements:
 *   - The free Dash IoT app (iOS / Android).
 *   - Wi-Fi credentials below, or provisioning from the app over BLE.
 *   - A dash.dashio.io account for the MQTT connection. BLE alone requires neither an account
 *     nor a network, so the example is usable with every credential left blank.
 *
 * Notes:
 *   - Trade-off: with Erqos IoT disabled, EQConnect provisioning, the Erqos cloud, and the
 *     library's own Home Assistant MQTT entities are unavailable. The Dash app provisions Wi-Fi
 *     instead. For EQConnect and Home Assistant, use the EQSP32 MQTT examples without DashIO.
 *   - Both connectivity stacks cannot manage the radios in one firmware image.
 *   - `DashioWiFi` calls `ESP.restart()` after `connectTimeoutS` failed connection attempts,
 *     which defaults to 300. This example sets it to 0, since an unannounced reboot would drop
 *     every output mid-operation on a controller driving real loads.
 *   - Serial output is on the USB console at 115200 baud.
 *
 * @author Erqos Technologies
 * @date 2026-09-08
 */

#include <EQSP32.h>

#include "DashioESP.h"
#include "DashioProvisionESP.h"

// -------------------- User settings --------------------
#define DEVICE_TYPE     "EQSP32"
#define DEVICE_NAME     "EQSP32 Dash Demo"

// Leave blank to provision Wi-Fi from the Dash app over BLE instead.
#define WIFI_SSID       ""
#define WIFI_PASSWORD   ""

// dash.dashio.io credentials. Leave blank to run BLE/TCP only.
#define DASH_USER       ""
#define DASH_PASSWORD   ""

// Outputs stay idle by default so this demo is safe to flash onto a board whose
// wiring is unknown. Set to 1 to let the dashboard actually drive P9.
#define DEMO_DRIVE_OUTPUT   0

// -------------------- Demo pin assignment --------------------
#define PIN_DIGITAL_IN      EQ_PIN_1    // Plain digital input
#define PIN_SWITCH_IN       EQ_PIN_2    // Debounced digital input (SWT)
#define PIN_ANALOG_IN       EQ_PIN_3    // Analog input, value in mV
#define PIN_POWER_OUT       EQ_PIN_9    // Low-side PWM output

#define SWITCH_DEBOUNCE_MS  50
#define UPDATE_PERIOD_MS    1000        // How often live values are pushed
#define HEARTBEAT_PERIOD_MS 500

// Full-scale of the analog dial. Must match the dial maximum in the embedded layout below.
#define ANALOG_MAX_MV       10000

// -------------------- Dash control IDs --------------------
// These must match the control IDs in the embedded layout below.
const char *BUTTON_OUTPUT_ID   = "B01";
const char *SLIDER_POWER_ID    = "SL01";
const char *TEXT_DIGITAL_IN_ID = "T01";
const char *TEXT_SWITCH_IN_ID  = "T02";
const char *TEXT_VIN_ID        = "T03";
const char *TEXT_VOUT_ID       = "T04";
const char *TEXT_UPTIME_ID     = "T05";
const char *DIAL_ANALOG_ID     = "D01";

// -------------------- Dashboard layout --------------------
// The Dash app decodes this and draws the controls itself, so nothing is placed by hand.
// It is the dashboard JSON, raw-deflated (zlib, wbits -15) and base64 encoded.
const char dashLayoutC64Str[] PROGMEM =
"1ZZRb9owEMe/CrK0t2hKQuk23ghJKVoKWeJlk6Y9mMSAhWMjxxllVb/7zknDWqCl6/awPoDOf9t3vt9d4tyg4cUI9W9QTn+w"
"jCZUV2vUR4IU1NqwObNyUi4tLwysq08YW3gYIQuJqvDr5SmjmxL1HQtl80VMf6C+e2shP02/oP63G5RJoZXkYx88+qntwFbN"
"NKcwDD4lUdcFodTbWvAGyXgI40xyqcyYk2wFYwZOJnAakCZS0Cb6UPKqEE3kckkUbQTU16qiFloolu+WuE4jxNIctduFEFrx"
"4V2YL0umjVOjeVLlVB2dWS1E3k74RK06CSeadkaKbO8vwYqIcg3nEdkW9e1mBpuUPXnd7seUcPRw6vF9F4AwYT8hfee8ka/I"
"9W+xa99+t5CH8eQAuFfzrp3qYxWYVnpd6U70odUiWTLNpDBHnEb3SnM5Hl2G8MNmJb3WID0sTCQ3VIE0q7SWIhBkxmneFkOK"
"NvGRolTAMjmf71iyooV43caPCfwDhbd2z0Lbo+qG5Xq5U84stKRssdQ7xekZLEnoxwdYkvAElzqZzpvHqXAGXbKXZElFPhV8"
"OxUx5ZSU4GhOeGlmlnJzxQRUrV1bMFHXuDCSY4OlaJ4SXpki29CtKyFn+104I2ofI0gXknNo66Q+Uusf9OSuckkwOkr23H0e"
"Wef9AdquIYuDr/iALD4B1mcLpgnvjEUnch7FWwmmy6bDTK8NOFuYyWEwwUEM4lyqgpgWnEwngYmnaMbK2gOQXM1yvF3T39MZ"
"lyX9OIPiJFCiFtFze809P91r9odbax+E+ySIZMN0tqw5uK+Dw5nzMg7dJzmkTJxMP31B/u5f5N/7p/mfPZ2/rPQrAdB74YPQ"
"exLA57VmBf3fHoHjBN49rwXg5eiPB+Hh58+Jl+NAEC4X9Tuhe/JCjsaH6e7fKvv3im07xy4j+JiaUXUv0o5uzgi/YJzvXztr"
"yYQ+/Exqi1Wkf3CX996fhuq6BmroGaZgfJxMvcbyx3FjXA2iOyOYfG6scDpqjEHqN0YSDvHd+mHYGpcxbiw8iqPLxvTwqHZ3"
"+ws=";

// -------------------- Objects --------------------
EQSP32 eqsp32;

DashDevice    dashDevice(DEVICE_TYPE, dashLayoutC64Str, 1);
DashProvision dashProvision(&dashDevice);

DashWiFi wifi;
DashBLE  ble_con(&dashDevice, true);
DashTCP  tcp_con(&dashDevice, true);
DashMQTT mqtt_con(&dashDevice, true, true);

EQTimer updateTimer(UPDATE_PERIOD_MS);
EQTimer heartbeatTimer(HEARTBEAT_PERIOD_MS);

// -------------------- Application state --------------------
static bool outputEnabled = false;      // Mirrors the dashboard button
static int  outputPowerPct = 50;        // Mirrors the dashboard slider

static uint32_t bootMillis = 0;


// -------------------- Helpers --------------------

/**
 * Sends a message back on the connection the request arrived on.
 */
static void sendMessage(ConnectionType connectionType, const String &message)
{
    if (connectionType == TCP_CONN)
        tcp_con.sendMessage(message);
    else if (connectionType == BLE_CONN)
        ble_con.sendMessage(message);
    else
        mqtt_con.sendMessage(message);
}


/**
 * Sends a message on every connection, so all attached dashboards stay in sync.
 */
static void sendMessageAll(const String &message)
{
    ble_con.sendMessage(message);
    tcp_con.sendMessage(message);
    mqtt_con.sendMessage(message);
}


static String uptimeString()
{
    uint32_t seconds = (millis() - bootMillis) / 1000;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%luh %02lum",
             (unsigned long)(seconds / 3600),
             (unsigned long)((seconds / 60) % 60));
    return String(buffer);
}


/**
 * Pushes the physical output state to the hardware.
 *
 * pinValue() takes 0-1000, the dashboard slider is a percentage. With
 * DEMO_DRIVE_OUTPUT at 0 the dashboard still tracks state, nothing is driven.
 */
static void applyOutput()
{
#if DEMO_DRIVE_OUTPUT
    eqsp32.pinValue(PIN_POWER_OUT, outputEnabled ? (outputPowerPct * 10) : 0);
#endif
}


/**
 * Builds the message carrying every live input value.
 */
static String buildValuesMessage()
{
    // Clamp into the dial's declared span. readPin() returns -1 if the pin could
    // not be configured (see the pinMode() warnings in setup()), and a value past
    // the dial maximum renders badly in the app rather than being ignored.
    int analog_mV = eqsp32.readPin(PIN_ANALOG_IN);
    if (analog_mV < 0)
        analog_mV = 0;
    else if (analog_mV > ANALOG_MAX_MV)
        analog_mV = ANALOG_MAX_MV;

    String message = dashDevice.getTextBoxMessage(
        TEXT_DIGITAL_IN_ID, eqsp32.readPin(PIN_DIGITAL_IN) ? "ON" : "OFF");
    message += dashDevice.getTextBoxMessage(
        TEXT_SWITCH_IN_ID, eqsp32.readPin(PIN_SWITCH_IN) ? "ON" : "OFF");
    message += dashDevice.getDialMessage(DIAL_ANALOG_ID, analog_mV);
    message += dashDevice.getTextBoxMessage(
        TEXT_VIN_ID, eqsp32.readInputVoltage() / 1000.0f);
    message += dashDevice.getTextBoxMessage(
        TEXT_VOUT_ID, eqsp32.readOutputVoltage() / 1000.0f);
    message += dashDevice.getTextBoxMessage(TEXT_UPTIME_ID, uptimeString());

    return message;
}


/**
 * Builds the message carrying the control states the dashboard owns.
 */
static String buildControlsMessage()
{
    String message = dashDevice.getButtonMessage(BUTTON_OUTPUT_ID, outputEnabled);
    message += dashDevice.getSliderMessage(SLIDER_POWER_ID, outputPowerPct);
    return message;
}


// -------------------- Dash message handling --------------------

/**
 * The app asks for "status" when a dashboard opens. Reply with the full
 * current state so every control shows a real value immediately.
 */
static void processStatus(ConnectionType connectionType)
{
    sendMessage(connectionType, buildControlsMessage());
    sendMessage(connectionType, buildValuesMessage());
}


static void processIncomingMessage(MessageData *messageData)
{
    switch (messageData->control)
    {
    case status:
        processStatus(messageData->connectionType);
        break;

    case button:
        if (messageData->idStr == BUTTON_OUTPUT_ID)
        {
            outputEnabled = !outputEnabled;
            applyOutput();
            eqsp32.buzzerOn(1500, 40);
            // Echo to every connection, not just the one that asked, so a
            // second phone watching the same device stays in step.
            sendMessageAll(dashDevice.getButtonMessage(BUTTON_OUTPUT_ID, outputEnabled));
        }
        break;

    case slider:
        if (messageData->idStr == SLIDER_POWER_ID)
        {
            outputPowerPct = messageData->payloadStr.toInt();
            if (outputPowerPct < 0)   outputPowerPct = 0;
            if (outputPowerPct > 100) outputPowerPct = 100;
            applyOutput();
            sendMessageAll(dashDevice.getSliderMessage(SLIDER_POWER_ID, outputPowerPct));
        }
        break;

    default:
        // Device name, Wi-Fi, TCP and MQTT setup messages from the app.
        dashProvision.processMessage(messageData);
        break;
    }
}


/**
 * Called when the app changes provisioning. If the comms settings changed we
 * have to restart the affected connections with the new credentials.
 */
static void onProvisionCallback(ConnectionType connectionType,
                                const String &message,
                                bool commsChanged)
{
    sendMessage(connectionType, message);

    if (commsChanged)
    {
        mqtt_con.setup(dashProvision.dashUserName, dashProvision.dashPassword);
        wifi.begin(dashProvision.wifiSSID, dashProvision.wifiPassword);
    }
    else
    {
        mqtt_con.sendWhoAnnounce();     // Publish the new device name
    }
}


// -------------------- Setup / loop --------------------

static void printBanner()
{
    Serial.println();
    Serial.println("============================================================");
    Serial.println("EQSP32 + Dash IoT demo");
    Serial.println("============================================================");
    Serial.print  ("EQSP32 device : ");
    Serial.println(eqsp32.getDeviceName());
    Serial.print  ("Dash deviceID : ");
    Serial.println(dashDevice.deviceID);
    Serial.print  ("Dash name     : ");
    Serial.println(dashDevice.name);
    Serial.printf ("Digital in    : P%d (DIN)\n", PIN_DIGITAL_IN);
    Serial.printf ("Switch in     : P%d (SWT, %d ms debounce)\n",
                   PIN_SWITCH_IN, SWITCH_DEBOUNCE_MS);
    Serial.printf ("Analog in     : P%d (AIN, mV)\n", PIN_ANALOG_IN);
#if DEMO_DRIVE_OUTPUT
    Serial.printf ("Power out     : P%d (POUT, driven by the dashboard)\n", PIN_POWER_OUT);
#else
    Serial.println("Power out     : disabled (set DEMO_DRIVE_OUTPUT to 1 to enable)");
#endif
    Serial.println("Erqos IoT     : disabled, DashIO owns Wi-Fi / BLE / MQTT");
    Serial.println("BLE LED       : on when a Dash app is connected over BLE");
    Serial.println("Wi-Fi LED     : blinks while connecting, solid once online");
    Serial.println("============================================================");
}


void setup()
{
    Serial.begin(115200);
    bootMillis = millis();

    // ---- EQSP32: hardware only -------------------------------------------
    EQSP32Configs configs;
    configs.userDevName = DEVICE_NAME;

    // The whole point of this demo. Without it, EQSP32's own connectivity runs
    // alongside DashIO's and the two compete for the same radios. See the
    // header comment.
    configs.disableErqosIoT = true;

    eqsp32.begin(configs, true);        // true enables the library's verbose output

    if (!eqsp32.pinMode(PIN_DIGITAL_IN, DIN))
        Serial.printf("WARNING: could not configure P%d as DIN\n", PIN_DIGITAL_IN);

    if (!eqsp32.pinMode(PIN_SWITCH_IN, SWT))
        Serial.printf("WARNING: could not configure P%d as SWT\n", PIN_SWITCH_IN);
    else
        eqsp32.configSWT(PIN_SWITCH_IN, SWITCH_DEBOUNCE_MS);

    if (!eqsp32.pinMode(PIN_ANALOG_IN, AIN))
        Serial.printf("WARNING: could not configure P%d as AIN\n", PIN_ANALOG_IN);

#if DEMO_DRIVE_OUTPUT
    if (!eqsp32.pinMode(PIN_POWER_OUT, POUT))
        Serial.printf("WARNING: could not configure P%d as POUT\n", PIN_POWER_OUT);
    applyOutput();
#endif

    // ---- DashIO: connectivity --------------------------------------------
    // The DashIO examples brace-initialise DeviceData:
    //
    //     DeviceData d = {DEVICE_NAME, WIFI_SSID, ...};
    //
    // That compiles in the Arduino IDE, which builds at gnu++17, and fails
    // here with "could not convert from <brace-enclosed initializer list>".
    // DeviceData has a default member initializer on tcpPort, which stops it
    // being an aggregate before C++14, and the ESP32 Arduino core builds at
    // gnu++11 under PlatformIO. Filling the fields explicitly compiles under
    // both, so this is worth copying into your own sketch rather than raising
    // the standard just for one struct.
    DeviceData defaultDeviceData = {};
    strncpy(defaultDeviceData.deviceName,   DEVICE_NAME,
            sizeof(defaultDeviceData.deviceName) - 1);
    strncpy(defaultDeviceData.wifiSSID,     WIFI_SSID,
            sizeof(defaultDeviceData.wifiSSID) - 1);
    strncpy(defaultDeviceData.wifiPassword, WIFI_PASSWORD,
            sizeof(defaultDeviceData.wifiPassword) - 1);
    strncpy(defaultDeviceData.dashUserName, DASH_USER,
            sizeof(defaultDeviceData.dashUserName) - 1);
    strncpy(defaultDeviceData.dashPassword, DASH_PASSWORD,
            sizeof(defaultDeviceData.dashPassword) - 1);
    defaultDeviceData.tcpPort = DEFAULT_TCP_PORT;

    dashProvision.load(&defaultDeviceData, &onProvisionCallback);

    // Reuse the EQSP32's own MAC for the Dash device ID, so the device keeps
    // its identity in the app across reflashes.
    dashDevice.setup(wifi.macAddress());

    ble_con.setCallback(&processIncomingMessage);
    ble_con.begin();

    // Never let a failed Wi-Fi connection reboot the controller.
    //
    // DashioWiFi counts failed connection attempts once a second and calls
    // ESP.restart() once the count passes connectTimeoutS, which defaults to
    // 300. That is a reasonable recovery for a bare dev board whose Wi-Fi stack
    // has locked up. It is the wrong default here for two reasons:
    //
    //   - With WIFI_SSID left blank, as shipped, the connection can never
    //     succeed, so the board reboots every 5 minutes even when you are only
    //     using BLE.
    //   - On an EQSP32 driving real loads, an unannounced reboot drops every
    //     output mid-operation. A controller should not restart itself because
    //     the network is down.
    //
    // 0 disables the reboot; Wi-Fi still keeps retrying in the background. If
    // you do want the watchdog-style recovery once real credentials are set,
    // raise it deliberately and make sure your outputs fail safe on reset.
    wifi.connectTimeoutS = 0;

    tcp_con.setCallback(&processIncomingMessage);
    wifi.attachConnection(&tcp_con);

    mqtt_con.setCallback(&processIncomingMessage);
    mqtt_con.setup(dashProvision.dashUserName, dashProvision.dashPassword);
    wifi.attachConnection(&mqtt_con);

    wifi.begin(dashProvision.wifiSSID, dashProvision.wifiPassword);

    printBanner();

    updateTimer.start();
    heartbeatTimer.start();
}


void loop()
{
    // DashIO is polled, not interrupt driven. Both of these must run every
    // pass or messages are delayed.
    ble_con.run();
    wifi.run();

    // Status LEDs are ours to drive because Erqos IoT is disabled.
    if (heartbeatTimer.isElapsed(true))
    {
        if (WiFi.status() == WL_CONNECTED)
            eqsp32.setWifiLed();
        else
            eqsp32.toggleWifiLed();

        if (ble_con.isConnected())
            eqsp32.setBleLed();
        else
            eqsp32.resetBleLed();
    }

    if (updateTimer.isElapsed(true))
        sendMessageAll(buildValuesMessage());

    delay(10);
}
