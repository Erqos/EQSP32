/**
 * @file EQSP32_DashIO_MultiScreen_Demo.ino
 * @brief Multi-screen Dash IoT dashboard for the EQSP32, covering every basic EQSP32 I/O mode
 *        and the main Dash control types across three device views.
 *
 * This example assumes:
 *   - Erqos IoT functionality is DISABLED (`disableErqosIoT = true`).
 *   - Wi-Fi, BLE, and MQTT are managed by the DashIO library, not by EQSP32.
 *   - The DashIO libraries are installed through the Arduino IDE Library Manager.
 *   - The dashboard layout is embedded below, so the Dash app draws all three views itself
 *     on first connect.
 *   - For the single-screen introduction, see EQSP32_DashIO_Basic_Demo.
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
 * Why Erqos IoT Is Disabled:
 *   - EQSP32 and DashIO are both complete IoT stacks, and each expects to own the device's
 *     BLE, Wi-Fi, and MQTT connectivity. Only one of them can.
 *   - BLE: EQSP32 uses BLE for EQConnect provisioning, and DashIO uses BLE to talk to the Dash
 *     app. Left enabled, the two compete for the radio and neither stays reliably discoverable.
 *   - Wi-Fi: EQSP32 manages the connection, including reconnect behavior and Ethernet failover.
 *     DashIO runs its own connection and reconnect logic. The two work against each other.
 *   - Setting `disableErqosIoT = true` hands connectivity to DashIO and resolves all of it.
 *   - Trade-off: EQConnect provisioning, the Erqos cloud, and the library's own Home Assistant
 *     MQTT entities become unavailable. The Dash app provisions Wi-Fi instead.
 *
 * Device Views:
 *   - DV01 "Control"  Two relay buttons, a PWM slider, a setpoint knob, a mode selector, and a
 *                     setpoint text entry.
 *   - DV02 "Monitor"  Every EQSP32 input mode, plus supply voltages and uptime.
 *   - DV03 "Trends"   A time graph of the analog input and temperature, and an event log.
 *
 * I/O Map:
 *   - P1   DIN     digital input
 *   - P2   SWT     debounced digital input
 *   - P3   AIN     analog input, mV
 *   - P4   RAIN    relative analog input, 0-1000 as per mille of the Vout reference
 *   - P5   TIN     NTC temperature, converted with CONVERT_TIN()
 *   - P6   CIN     current input, mA
 *   - P9   RELAY   relay 1, pull-in then hold power
 *   - P10  RELAY   relay 2
 *   - P11  POUT    low-side PWM, driven by the slider
 *   - P12  PCC     pulse counter, destructive read
 *   Two opposite pin constraints apply. Analog modes (AIN, RAIN, TIN, CIN) are available on
 *   terminals 1-8 only. PCC is the reverse: terminals 9-16 only, and at most 4 at once.
 *   DIN, SWT, POUT and RELAY work on all 16.
 *
 *   Hence the allocation rule this demo follows: put outputs on terminals 9-16 and keep 1-8
 *   for inputs. POUT and RELAY work anywhere, so spending an analog-capable terminal on an
 *   output wastes one of only eight that can ever read a sensor. Here P7 and P8 are left free
 *   precisely because they are analog-capable and something later may need them.
 *
 * Hardware Setup:
 *   - One EQSP32 controller. No expansion modules required.
 *   - Inputs may be left unconnected; they simply read at rest.
 *   - Outputs stay idle unless the flags below are set to 1. Confirm what is wired to P9, P10,
 *     and P11 before enabling them.
 *
 * Requirements:
 *   - The free Dash IoT app (iOS / Android).
 *   - Wi-Fi credentials below, or provisioning from the app over BLE.
 *   - A dash.dashio.io account for the MQTT connection. BLE alone needs neither an account nor
 *     a network, so the example is usable with every credential left blank.
 *
 * Notes:
 *   - `DashioWiFi` calls `ESP.restart()` after `connectTimeoutS` failed connection attempts,
 *     which defaults to 300. This example sets it to 0, since an unannounced reboot would drop
 *     every output mid-operation on a controller driving real loads.
 *   - The event log needs a timestamp. With no network there is no NTP time, so timestamps fall
 *     back to a synthetic clock based on uptime. They are well formed but not real dates.
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
#define DEVICE_NAME     "EQSP32 Dash MultiScreen"

// Leave blank to provision Wi-Fi from the Dash app over BLE instead.
#define WIFI_SSID       ""
#define WIFI_PASSWORD   ""

// dash.dashio.io credentials. Leave blank to run BLE/TCP only.
#define DASH_USER       ""
#define DASH_PASSWORD   ""

// Outputs stay idle by default so this demo is safe to flash onto a board whose wiring is
// unknown. The relays are gated separately from the PWM output: enabling relays makes real
// contacts switch, which is worth an explicit decision of its own.
#define DEMO_DRIVE_OUTPUT   0       // 1 lets the slider drive P11 (POUT)
#define DEMO_DRIVE_RELAYS   0       // 1 lets the buttons drive P9 / P10 (RELAY)

// -------------------- Demo pin assignment --------------------
#define PIN_DIGITAL_IN      EQ_PIN_1    // DIN
#define PIN_SWITCH_IN       EQ_PIN_2    // SWT, debounced
#define PIN_ANALOG_IN       EQ_PIN_3    // AIN, mV
#define PIN_RELATIVE_IN     EQ_PIN_4    // RAIN, 0-1000
#define PIN_TEMP_IN         EQ_PIN_5    // TIN, NTC
#define PIN_CURRENT_IN      EQ_PIN_6    // CIN, mA x 100
#define PIN_PULSE_IN        EQ_PIN_12   // PCC, destructive read (PCC needs terminal 9-16)
#define PIN_RELAY_1         EQ_PIN_9    // RELAY
#define PIN_RELAY_2         EQ_PIN_10   // RELAY
#define PIN_POWER_OUT       EQ_PIN_11   // POUT, PWM

#define SWITCH_DEBOUNCE_MS  50
#define RELAY_HOLD_POWER    300         // 30% holding power after pull-in
#define RELAY_DERATE_MS     1000        // full power for this long, then hold

#define UPDATE_PERIOD_MS    1000        // live values pushed at this rate
#define TREND_PERIOD_MS     10000       // time-graph point added at this rate
#define HEARTBEAT_PERIOD_MS 500

// Full-scale values. Keep in step with the embedded layout below.
#define ANALOG_MAX_MV       10000
#define RAIN_MAX            1000
#define SETPOINT_MAX        100

// -------------------- Dash control IDs --------------------
// These must match the control IDs in the embedded layout below.
const char *BTN_RELAY1_ID    = "B01";
const char *BTN_RELAY2_ID    = "B02";
const char *SLIDER_PWM_ID    = "SL01";
const char *KNOB_SETPOINT_ID = "K01";
const char *SELECTOR_MODE_ID = "S01";
const char *TEXT_SETPOINT_ID = "T10";

const char *TEXT_DIN_ID      = "T01";
const char *TEXT_SWT_ID      = "T02";
const char *DIAL_AIN_ID      = "D01";
const char *DIAL_RAIN_ID     = "D02";
const char *TEXT_TIN_ID      = "T03";
const char *TEXT_CIN_ID      = "T04";
const char *TEXT_PCC_ID      = "T05";
const char *TEXT_VIN_ID      = "T06";
const char *TEXT_VOUT_ID     = "T07";
const char *TEXT_UPTIME_ID   = "T08";

const char *TGRAPH_ID        = "TG01";
const char *EVENTLOG_ID      = "EL01";

// -------------------- Dashboard layout --------------------
// The Dash app decodes this and draws all three views itself, so nothing is placed
// by hand. It is the dashboard JSON, raw-deflated (zlib, wbits -15) and base64
// encoded.
const char dashLayoutC64Str[] PROGMEM =
"7Zjdbuo4EIBfBUXau2iXBOhpuYNAKSp/m7h0pdVeGGLAqmMjx2lhq777jhPCYZNA0m616pHOBcieccbjz+PJOK+Gczsw2q+G"
"T57pknhERVujbXAcEPOFrqjp43Bjdkd9c/w7QiZyZoZp8CjoxcPnlLyERrthGsvV2iXPRtt+M43efP5otP98NZaCKynYsAcW"
"e/O6BY8qqhiBrpOoQBKqfSzpdryhA/2lYELqPsPLJ+hTsDIBd0A0Ey9EJvM7gkUBh7ktsLDBkiQCo61kRExjLal/HGJbicAV"
"sbPaWyWZc5jncUMVMRJZV0ifyELN05r7qaKH5VPNY1iR2kDi/ekQJDEPt+APX+6Ndj3RIL3ortilzyOCmfFv1fnnboGUR/+G"
"9VtXiXiMd9+F9vWbmSNtn5AeC06VkO8j/bD9ibkcc+MEMwKbfvg+yogG5CfnLOe/TKOL0CSXQbpxAomNqqKU4hKG9zWrVpvd"
"pNKZCKmigmsnp7OTzbkbDu5G8EN6JNkpEBWnmkWklOB9jheM+Ol2CJ4ufSAJ4TBMrFZHmjRIMe7S+V0M/8Dh13rLNPaF0hfq"
"q81R0jSNDaHrjTpKrGY2ALvxMS/jYQMPq/5lgbT+CxCIFG/Uc3OR4o1KQmX2OAYqVu2X81wYhaOTWWYIR3zK2X7KgS3BIZha"
"YRZqzUa8jCmHUE7HBpTHgR9okVWHliT+HLNIR34djvATF4vs0VxgmQUJolvBGJx1L3YptQ9y77B3Xn9QHGzXRWztqyxbfRAz"
"cBsx3PvJtJuDe1/CFkqIraBclYbcZOqOOyOjEOuB9+dQ9SlmB4b3oE5taXGWdh5ioyLERj5C7UOEOqggREsojoVPzhKs6mXr"
"uoKX9RvtJer/gXJOojhxlG91DfRyf9bdCAqQMEkoOrV0GF1rpdOfoL4LwpWQAVZxREz6ekJJljSMLcAGPy18tN+S7+olEyG5"
"X0DIeBA46W5WZfLNrsYkk2xRwX6dVlm94UQn2q/G4Mz7plmeXuvXeQT2RQTeIwIE9ldD0PpUBI2LCFAcBa1SBM4HGFifHgbN"
"DzJoXmTgxAyuShkEnQ9AsD89ED4KoXURwiyCsiDUGcH+MVJC/lVRDcPVRQxzykuXP/8SYfDR9X+7vH4RqS8HoDgA8u/FagCu"
"LwJ42KrkjvsDvBIqEoBiqTeEujX3gaukQujEebFRWhbPhvmVZsvebOFbr1tF1TKPggWRJzMdwcYlMWW5+jcu5/KfDI45e/6O"
"W611Vc7Tzt1qeyVVhptwbH4OxwzG/4fiNnjHVbgiRV3CD9zZXb6GHxTE5emHqw7HTKxrv9UQCbZEYhXJ8wd239nRcIQXhIGI"
"kZVKZeMD2aRzvJ3F3UkUdLGElbdODbj6XEu9jhMbWnhiRXdjOwUx16j24eAmj6tpa1yj6SAHqz8qgdV/BkX4jotZsZutZhU3"
"Yy+7OtPEOcdNGuPO7NDoTx6SVmfeSxrO6DDIuXNR0uqiQTz+7R8=";

// -------------------- Objects --------------------
EQSP32 eqsp32;

DashDevice    dashDevice(DEVICE_TYPE, dashLayoutC64Str, 3);
DashProvision dashProvision(&dashDevice);

DashWiFi wifi;
DashBLE  ble_con(&dashDevice, true);
DashTCP  tcp_con(&dashDevice, true);
DashMQTT mqtt_con(&dashDevice, true, true);

EQTimer updateTimer(UPDATE_PERIOD_MS);
EQTimer trendTimer(TREND_PERIOD_MS);
EQTimer heartbeatTimer(HEARTBEAT_PERIOD_MS);

// -------------------- Application state --------------------
static bool relay1On = false;
static bool relay2On = false;
static int  pwmPercent = 0;
static int  setpoint = 50;
static int  modeIndex = 0;              // 0 Off, 1 Manual, 2 Auto

static uint32_t pulseTotal = 0;         // PCC reads are destructive, so accumulate
static uint32_t bootMillis = 0;
static bool prevUserButton = false;

static const char *MODE_NAMES[] = {"Off", "Manual", "Auto"};
static const int   MODE_COUNT   = 3;


// -------------------- Helpers --------------------

static void sendMessage(ConnectionType connectionType, const String &message)
{
    if (connectionType == TCP_CONN)
        tcp_con.sendMessage(message);
    else if (connectionType == BLE_CONN)
        ble_con.sendMessage(message);
    else
        mqtt_con.sendMessage(message);
}


static void sendMessageAll(const String &message)
{
    ble_con.sendMessage(message);
    tcp_con.sendMessage(message);
    mqtt_con.sendMessage(message);
}


static uint32_t uptimeSeconds()
{
    return (millis() - bootMillis) / 1000;
}


static String uptimeString()
{
    uint32_t s = uptimeSeconds();
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%luh %02lum",
             (unsigned long)(s / 3600), (unsigned long)((s / 60) % 60));
    return String(buffer);
}


/**
 * Timestamp for event-log entries.
 *
 * Uses real UTC time when NTP has set the clock. Over BLE there is no network and no NTP, so
 * this falls back to a synthetic clock counting up from a fixed date. The format is what the
 * Dash app expects; only the absolute date is meaningless in that case.
 */
static String logTimestamp()
{
    struct tm dt;
    char buffer[24];

    if (getLocalTime(&dt, 0))
    {
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 1900 + dt.tm_year, 1 + dt.tm_mon, dt.tm_mday,
                 dt.tm_hour, dt.tm_min, dt.tm_sec);
        return String(buffer);
    }

    uint32_t s = uptimeSeconds();
    snprintf(buffer, sizeof(buffer), "2026-01-01T%02lu:%02lu:%02luZ",
             (unsigned long)((s / 3600) % 24),
             (unsigned long)((s / 60) % 60),
             (unsigned long)(s % 60));
    return String(buffer);
}


/**
 * Adds one line to the Dash event log on every connection.
 */
static void logEvent(const String &text, const String &colour = "blue")
{
    String lines[1];
    lines[0] = text;

    String message = "";
    dashDevice.addEventLogMessage(message, EVENTLOG_ID, logTimestamp(), colour, lines, 1);
    sendMessageAll(message);
}


// -------------------- Hardware output --------------------

/**
 * Pushes relay state to the hardware.
 *
 * pinValue() sets the pull-in power; configRELAY() drops it to the holding power after the
 * derate delay, so the coil is not held at full power. With DEMO_DRIVE_RELAYS at 0 the
 * dashboard still tracks state and nothing switches.
 */
static void applyRelays()
{
#if DEMO_DRIVE_RELAYS
    eqsp32.pinValue(PIN_RELAY_1, relay1On ? 1000 : 0);
    eqsp32.pinValue(PIN_RELAY_2, relay2On ? 1000 : 0);
#endif
}


/**
 * Pushes PWM duty to the hardware. pinValue() takes 0-1000, the slider is a percentage.
 */
static void applyOutput()
{
#if DEMO_DRIVE_OUTPUT
    eqsp32.pinValue(PIN_POWER_OUT, pwmPercent * 10);
#endif
}


// -------------------- Dash message building --------------------

/**
 * Control states the dashboard owns, echoed back so every attached phone agrees.
 */
static String buildControlsMessage()
{
    String selection[MODE_COUNT];
    for (int i = 0; i < MODE_COUNT; i++)
        selection[i] = MODE_NAMES[i];

    String message = dashDevice.getButtonMessage(BTN_RELAY1_ID, relay1On);
    message += dashDevice.getButtonMessage(BTN_RELAY2_ID, relay2On);
    message += dashDevice.getSliderMessage(SLIDER_PWM_ID, pwmPercent);
    message += dashDevice.getKnobMessage(KNOB_SETPOINT_ID, setpoint);
    message += dashDevice.getSelectorMessage(SELECTOR_MODE_ID, modeIndex,
                                             selection, MODE_COUNT);
    message += dashDevice.getTextBoxMessage(TEXT_SETPOINT_ID, String(setpoint));
    return message;
}


/**
 * Every live input reading.
 */
static String buildValuesMessage()
{
    int analog_mV = eqsp32.readPin(PIN_ANALOG_IN);
    if (analog_mV < 0) analog_mV = 0;
    else if (analog_mV > ANALOG_MAX_MV) analog_mV = ANALOG_MAX_MV;

    int relative = eqsp32.readPin(PIN_RELATIVE_IN);
    if (relative < 0) relative = 0;
    else if (relative > RAIN_MAX) relative = RAIN_MAX;

    // PCC clears its counter on every read, so accumulate rather than display the raw read.
    int pulses = eqsp32.readPin(PIN_PULSE_IN);
    if (pulses > 0)
        pulseTotal += (uint32_t)pulses;

    String message = dashDevice.getTextBoxMessage(
        TEXT_DIN_ID, eqsp32.readPin(PIN_DIGITAL_IN) ? "ON" : "OFF");
    message += dashDevice.getTextBoxMessage(
        TEXT_SWT_ID, eqsp32.readPin(PIN_SWITCH_IN) ? "ON" : "OFF");
    message += dashDevice.getDialMessage(DIAL_AIN_ID, analog_mV);
    message += dashDevice.getDialMessage(DIAL_RAIN_ID, relative);
    message += dashDevice.getTextBoxMessage(
        TEXT_TIN_ID, CONVERT_TIN(eqsp32.readPin(PIN_TEMP_IN)));
    message += dashDevice.getTextBoxMessage(
        TEXT_CIN_ID, eqsp32.readPin(PIN_CURRENT_IN) / 100.0f);
    message += dashDevice.getTextBoxMessage(TEXT_PCC_ID, String(pulseTotal));
    message += dashDevice.getTextBoxMessage(
        TEXT_VIN_ID, eqsp32.readInputVoltage() / 1000.0f);
    message += dashDevice.getTextBoxMessage(
        TEXT_VOUT_ID, eqsp32.readOutputVoltage() / 1000.0f);
    message += dashDevice.getTextBoxMessage(TEXT_UPTIME_ID, uptimeString());

    return message;
}


// -------------------- Dash message handling --------------------

/**
 * The app asks for "status" when a dashboard opens. Reply with the full current state, plus
 * the time-graph line definitions so the graph has named series to plot into.
 */
static void processStatus(ConnectionType connectionType)
{
    sendMessage(connectionType, buildControlsMessage());
    sendMessage(connectionType, buildValuesMessage());

    String message = dashDevice.getTimeGraphLine(TGRAPH_ID, "l1", "Analog %",
                                                 line, "green", yLeft);
    message += dashDevice.getTimeGraphLine(TGRAPH_ID, "l2", "Temp C",
                                           line, "yellow", yLeft);
    sendMessage(connectionType, message);
}


static void processButton(MessageData *messageData)
{
    if (messageData->idStr == BTN_RELAY1_ID)
    {
        relay1On = !relay1On;
        applyRelays();
        sendMessageAll(dashDevice.getButtonMessage(BTN_RELAY1_ID, relay1On));
        logEvent(String("Relay 1 ") + (relay1On ? "ON" : "OFF"),
                 relay1On ? "green" : "gray");
    }
    else if (messageData->idStr == BTN_RELAY2_ID)
    {
        relay2On = !relay2On;
        applyRelays();
        sendMessageAll(dashDevice.getButtonMessage(BTN_RELAY2_ID, relay2On));
        logEvent(String("Relay 2 ") + (relay2On ? "ON" : "OFF"),
                 relay2On ? "green" : "gray");
    }
}


static void setSetpoint(int value, ConnectionType connectionType)
{
    if (value < 0) value = 0;
    else if (value > SETPOINT_MAX) value = SETPOINT_MAX;
    setpoint = value;

    // Knob and text box show the same number, so both are refreshed together.
    String message = dashDevice.getKnobMessage(KNOB_SETPOINT_ID, setpoint);
    message += dashDevice.getTextBoxMessage(TEXT_SETPOINT_ID, String(setpoint));
    sendMessageAll(message);
    (void)connectionType;
}


static void processIncomingMessage(MessageData *messageData)
{
    switch (messageData->control)
    {
    case status:
        processStatus(messageData->connectionType);
        break;

    case button:
        processButton(messageData);
        break;

    case slider:
        if (messageData->idStr == SLIDER_PWM_ID)
        {
            pwmPercent = messageData->payloadStr.toInt();
            if (pwmPercent < 0) pwmPercent = 0;
            else if (pwmPercent > 100) pwmPercent = 100;
            applyOutput();
            sendMessageAll(dashDevice.getSliderMessage(SLIDER_PWM_ID, pwmPercent));
        }
        break;

    case knob:
        if (messageData->idStr == KNOB_SETPOINT_ID)
            setSetpoint(messageData->payloadStr.toInt(), messageData->connectionType);
        break;

    case textBox:
        if (messageData->idStr == TEXT_SETPOINT_ID)
            setSetpoint(messageData->payloadStr.toInt(), messageData->connectionType);
        break;

    case selector:
        if (messageData->idStr == SELECTOR_MODE_ID)
        {
            modeIndex = messageData->payloadStr.toInt();
            if (modeIndex < 0 || modeIndex >= MODE_COUNT)
                modeIndex = 0;
            sendMessageAll(dashDevice.getSelectorMessage(SELECTOR_MODE_ID, modeIndex));
            logEvent(String("Mode: ") + MODE_NAMES[modeIndex], "blue");
        }
        break;

    default:
        // Device name, Wi-Fi, TCP and MQTT setup messages from the app.
        dashProvision.processMessage(messageData);
        break;
    }
}


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
        mqtt_con.sendWhoAnnounce();
    }
}


// -------------------- Setup / loop --------------------

static void configurePin(int pin, EQ_PinMode mode, const char *label)
{
    if (!eqsp32.pinMode(pin, mode))
        Serial.printf("WARNING: could not configure P%d as %s\n", pin, label);
}


static void printBanner()
{
    Serial.println();
    Serial.println("============================================================");
    Serial.println("EQSP32 + Dash IoT multi-screen demo");
    Serial.println("============================================================");
    Serial.print  ("Dash deviceID : ");
    Serial.println(dashDevice.deviceID);
    Serial.print  ("Dash name     : ");
    Serial.println(dashDevice.name);
    Serial.println("Views         : Control / Monitor / Trends");
    Serial.printf ("Inputs        : P%d DIN, P%d SWT, P%d AIN, P%d RAIN, P%d TIN, P%d CIN, P%d PCC\n",
                   PIN_DIGITAL_IN, PIN_SWITCH_IN, PIN_ANALOG_IN, PIN_RELATIVE_IN,
                   PIN_TEMP_IN, PIN_CURRENT_IN, PIN_PULSE_IN);
#if DEMO_DRIVE_RELAYS
    Serial.printf ("Relays        : P%d, P%d (LIVE)\n", PIN_RELAY_1, PIN_RELAY_2);
#else
    Serial.printf ("Relays        : P%d, P%d (disabled, set DEMO_DRIVE_RELAYS to 1)\n",
                   PIN_RELAY_1, PIN_RELAY_2);
#endif
#if DEMO_DRIVE_OUTPUT
    Serial.printf ("PWM out       : P%d (LIVE)\n", PIN_POWER_OUT);
#else
    Serial.printf ("PWM out       : P%d (disabled, set DEMO_DRIVE_OUTPUT to 1)\n",
                   PIN_POWER_OUT);
#endif
    Serial.println("Erqos IoT     : disabled, DashIO owns Wi-Fi / BLE / MQTT");
    Serial.println("============================================================");
}


void setup()
{
    Serial.begin(115200);
    bootMillis = millis();

    // ---- EQSP32: hardware only ----
    EQSP32Configs configs;
    configs.userDevName = DEVICE_NAME;
    configs.relaySequencer = true;      // stagger relay switching to limit inrush
    configs.disableErqosIoT = true;     // DashIO owns Wi-Fi, BLE and MQTT

    eqsp32.begin(configs, true);

    // Inputs. Analog modes are only available on terminals 1-8.
    configurePin(PIN_DIGITAL_IN,  DIN,  "DIN");
    configurePin(PIN_SWITCH_IN,   SWT,  "SWT");
    eqsp32.configSWT(PIN_SWITCH_IN, SWITCH_DEBOUNCE_MS);
    configurePin(PIN_ANALOG_IN,   AIN,  "AIN");
    configurePin(PIN_RELATIVE_IN, RAIN, "RAIN");
    configurePin(PIN_TEMP_IN,     TIN,  "TIN");
    configurePin(PIN_CURRENT_IN,  CIN,  "CIN");
    if (eqsp32.pinMode(PIN_PULSE_IN, PCC))
        eqsp32.configPCC(PIN_PULSE_IN, ON_RISING);
    else
        Serial.printf("WARNING: could not configure P%d as PCC\n", PIN_PULSE_IN);

    // Outputs.
#if DEMO_DRIVE_RELAYS
    configurePin(PIN_RELAY_1, RELAY, "RELAY");
    configurePin(PIN_RELAY_2, RELAY, "RELAY");
    eqsp32.configRELAY(PIN_RELAY_1, RELAY_HOLD_POWER, RELAY_DERATE_MS);
    eqsp32.configRELAY(PIN_RELAY_2, RELAY_HOLD_POWER, RELAY_DERATE_MS);
    applyRelays();
#endif
#if DEMO_DRIVE_OUTPUT
    configurePin(PIN_POWER_OUT, POUT, "POUT");
    applyOutput();
#endif

    // ---- DashIO: connectivity ----
    // The DashIO examples brace-initialise DeviceData, which compiles at gnu++17 in the Arduino
    // IDE but fails at gnu++11 under PlatformIO. Filling the fields explicitly works in both.
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

    dashDevice.setup(wifi.macAddress());

    // Never let a failed Wi-Fi connection reboot the controller. DashioWiFi restarts the ESP32
    // after connectTimeoutS failed attempts, which would drop every output mid-operation.
    wifi.connectTimeoutS = 0;

    ble_con.setCallback(&processIncomingMessage);
    ble_con.begin();

    tcp_con.setCallback(&processIncomingMessage);
    wifi.attachConnection(&tcp_con);

    mqtt_con.setCallback(&processIncomingMessage);
    mqtt_con.setup(dashProvision.dashUserName, dashProvision.dashPassword);
    mqtt_con.addDashStore(timeGraph, TGRAPH_ID);
    mqtt_con.addDashStore(eventLog, EVENTLOG_ID);
    wifi.attachConnection(&mqtt_con);

    wifi.begin(dashProvision.wifiSSID, dashProvision.wifiPassword);

    printBanner();

    updateTimer.start();
    trendTimer.start();
    heartbeatTimer.start();
}


void loop()
{
    // DashIO is polled, not interrupt driven. Both must run every pass.
    ble_con.run();
    wifi.run();

    // Short beep and a log entry on a user-button press.
    bool userButton = eqsp32.readUserButton();
    if (userButton && !prevUserButton)
    {
        eqsp32.buzzerOn(1500, 60);
        logEvent("User button pressed", "purple");
    }
    prevUserButton = userButton;

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

    if (trendTimer.isElapsed(true))
    {
        // Analog input as a percentage of full scale, so both series share one axis.
        int analog_mV = eqsp32.readPin(PIN_ANALOG_IN);
        if (analog_mV < 0) analog_mV = 0;
        float analogPercent = (float)analog_mV * 100.0f / (float)ANALOG_MAX_MV;

        String message = dashDevice.getTimeGraphPoint(TGRAPH_ID, "l1", analogPercent);
        message += dashDevice.getTimeGraphPoint(TGRAPH_ID, "l2",
                                                CONVERT_TIN(eqsp32.readPin(PIN_TEMP_IN)));
        sendMessageAll(message);
    }

    delay(10);
}
