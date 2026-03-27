/**
 * @file EQSP32_WiFi_BLE_MQTT_Runtime_Control_Demo.ino
 * @brief Demonstrates runtime control APIs for Wi-Fi, BLE, MQTT, and device name, along with live status monitoring.
 *
 * This example assumes:
 *   - Erqos IoT functionality is enabled.
 *   - Wi-Fi and BLE are managed by the EQSP32 library.
 *   - Commands are entered through the USB serial monitor at 115200 baud.
 *
 * Demonstrated APIs:
 *   - `startWiFi()` starts Wi-Fi using the stored credentials.
 *   - `startWiFi(newSSID, newPASS)` starts Wi-Fi using new credentials provided from serial.
 *   - `stopWiFi()` stops the Wi-Fi interface.
 *   - `clearWiFi()` clears the stored Wi-Fi credentials from flash memory.
 *   - `startBle()` starts BLE advertising.
 *   - `stopBle()` stops BLE advertising when possible.
 *   - `updateMQTT(user, pass, true)` updates MQTT credentials and enables MQTT.
 *   - `updateMQTT(true)` enables MQTT without changing stored credentials.
 *   - `updateMQTT(false)` disables MQTT without changing stored credentials.
 *   - `setDeviceName()` changes the EQSP32 device name at runtime.
 *
 * Status Monitoring:
 *   - Uses `getWiFiStatus()` to monitor the Wi-Fi connection state.
 *   - Uses `getBleStatus()` to monitor the BLE state.
 *   - Uses `getMQTTStatus()` to monitor the MQTT broker connection state.
 *   - Also reports the device online state and current local IP address.
 *
 * Command Interface:
 *   - The example includes a simple serial command parser.
 *   - Commands allow the developer to start or stop Wi-Fi, clear Wi-Fi credentials,
 *     start or stop BLE advertising, enable or disable MQTT, update MQTT credentials,
 *     change the device name, and request the current status.
 *
 * Notes:
 *   - `wifi_start <ssid> <password>` and `mqtt_on <user> <password>` use simple space-separated parsing.
 *   - For this reason, SSID, password, username, and password values should not contain spaces in this example.
 *
 * @author Erqos Technologies
 * @date 2026-03-24
 */

#include <EQSP32.h>

void printHelp();
void printWiFiStatus(EQ_WifiStatus status);
void printBleStatus(EQ_BleStatus status);
void printMQTTStatus(EQ_MQTTBrokerStatus status);
void printFullStatus();
void monitorStatusChanges();

String trimCopy(const String& s);
String getFirstToken(String& line);

void processCommand(String line);
void handleSerialCommands();


EQSP32 eqsp32;

// Previous status trackers
static EQ_WifiStatus prevWiFiStatus       = (EQ_WifiStatus)(-1);
static EQ_BleStatus prevBleStatus         = EQ_BLE_NO_INIT;
static EQ_MQTTBrokerStatus prevMQTTStatus = EQ_MQTT_NO_INIT;
static String prevLocalIP                 = "";
static bool prevOnline                    = false;

// Serial command buffer
static String serialLine = "";

void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println("EQSP32 Runtime API Functions Demo");
    Serial.println("---------------------------------");
    Serial.println("This example demonstrates runtime control of WiFi, BLE, MQTT and device name.");
    Serial.println("Type 'help' to see available commands.");
    Serial.println();

    eqsp32.begin();

    printHelp();
    printFullStatus();

    prevWiFiStatus = eqsp32.getWiFiStatus();
    prevBleStatus = eqsp32.getBleStatus();
    prevMQTTStatus = eqsp32.getMQTTStatus();
    prevOnline = eqsp32.isDeviceOnline();
    prevLocalIP = eqsp32.localIP();
}

void loop()
{
    handleSerialCommands();
    monitorStatusChanges();
    delay(10);
}



void monitorStatusChanges()
{
    EQ_WifiStatus wiFiStatus = eqsp32.getWiFiStatus();
    EQ_BleStatus bleStatus = eqsp32.getBleStatus();
    EQ_MQTTBrokerStatus mqttStatus = eqsp32.getMQTTStatus();
    bool online = eqsp32.isDeviceOnline();
    String localIP = eqsp32.localIP();

    if (wiFiStatus != prevWiFiStatus)
    {
        Serial.print("[STATUS] ");
        printWiFiStatus(wiFiStatus);
        prevWiFiStatus = wiFiStatus;
    }

    if (bleStatus != prevBleStatus)
    {
        Serial.print("[STATUS] ");
        printBleStatus(bleStatus);
        prevBleStatus = bleStatus;
    }

    if (mqttStatus != prevMQTTStatus)
    {
        Serial.print("[STATUS] ");
        printMQTTStatus(mqttStatus);
        prevMQTTStatus = mqttStatus;
    }

    if (online != prevOnline)
    {
        Serial.print("[STATUS] Device Online: ");
        Serial.println(online ? "Yes" : "No");
        prevOnline = online;
    }

    if (localIP != prevLocalIP)
    {
        Serial.print("[STATUS] Local IP: ");
        Serial.println(localIP);
        prevLocalIP = localIP;
    }
}

void processCommand(String line)
{
    line.trim();
    if (line.length() == 0)
        return;

    String command = getFirstToken(line);

    if (command.equalsIgnoreCase("help"))
    {
        printHelp();
    }
    else if (command.equalsIgnoreCase("status"))
    {
        printFullStatus();
    }
    else if (command.equalsIgnoreCase("wifi_start"))
    {
        if (line.length() == 0)
        {
            Serial.println("[CMD] startWiFi() with stored credentials");
            if (eqsp32.startWiFi())
                Serial.println("WiFi start requested using stored credentials.");
            else
                Serial.println("WiFi could not start.");
        }
        else
        {
            String ssid = getFirstToken(line);
            String pass = trimCopy(line);

            Serial.print("[CMD] startWiFi(\"");
            Serial.print(ssid);
            Serial.println("\", <password>)");

            if (eqsp32.startWiFi(ssid, pass))
                Serial.println("WiFi start requested using provided credentials.");
            else
                Serial.println("WiFi could not start with the provided credentials.");
        }
    }
    else if (command.equalsIgnoreCase("wifi_stop"))
    {
        Serial.println("[CMD] stopWiFi()");
        if (eqsp32.stopWiFi())
            Serial.println("WiFi stopped successfully.");
        else
            Serial.println("WiFi was not active or could not be stopped.");
    }
    else if (command.equalsIgnoreCase("wifi_clear"))
    {
        Serial.println("[CMD] clearWiFi()");
        if (eqsp32.clearWiFi())
            Serial.println("Stored WiFi credentials cleared.");
        else
            Serial.println("WiFi credentials could not be cleared.");
    }
    else if (command.equalsIgnoreCase("ble_start"))
    {
        Serial.println("[CMD] startBle()");
        if (eqsp32.startBle())
            Serial.println("BLE advertising started.");
        else
            Serial.println("BLE was already active or could not be started.");
    }
    else if (command.equalsIgnoreCase("ble_stop"))
    {
        Serial.println("[CMD] stopBle()");
        if (eqsp32.stopBle())
            Serial.println("BLE advertising stopped.");
        else
            Serial.println("BLE was not advertising or could not be stopped.");
    }
    else if (command.equalsIgnoreCase("mqtt_on"))
    {
        if (line.length() == 0)
        {
            Serial.println("[CMD] updateMQTT(true)");
            eqsp32.updateMQTT(true);
            Serial.println("MQTT enabled without modifying stored credentials.");
        }
        else
        {
            String user = getFirstToken(line);
            String pass = trimCopy(line);

            Serial.print("[CMD] updateMQTT(\"");
            Serial.print(user);
            Serial.println("\", <password>, true)");

            eqsp32.updateMQTT(user, pass, true);
            Serial.println("MQTT credentials updated and MQTT enabled.");
        }
    }
    else if (command.equalsIgnoreCase("mqtt_off"))
    {
        Serial.println("[CMD] updateMQTT(false)");
        eqsp32.updateMQTT(false);
        Serial.println("MQTT disabled without modifying stored credentials.");
    }
    else if (command.equalsIgnoreCase("name"))
    {
        String newName = trimCopy(line);

        if (newName.length() == 0)
        {
            Serial.println("Usage: name <new device name>");
            return;
        }

        Serial.print("[CMD] setDeviceName(\"");
        Serial.print(newName);
        Serial.println("\")");
        eqsp32.setDeviceName(newName);
        Serial.println("Device name updated.");
    }
    else
    {
        Serial.println("Unknown command. Type 'help' for the list of commands.");
    }
}

void handleSerialCommands()
{
    while (Serial.available() > 0)
    {
        char c = (char)Serial.read();

        if (c == '\r')
            continue;

        if (c == '\n')
        {
            processCommand(serialLine);
            serialLine = "";
        }
        else
        {
            serialLine += c;
        }
    }
}


/** ***********************************************
                  Helper Functions
****************************************************/
void printHelp()
{
    Serial.println();
    Serial.println("============= EQSP32 Runtime API Demo =============");
    Serial.println("help");
    Serial.println("status");
    Serial.println();
    Serial.println("WiFi:");
    Serial.println("wifi_start");
    Serial.println("wifi_start <ssid> <password>");
    Serial.println("wifi_stop");
    Serial.println("wifi_clear");
    Serial.println();
    Serial.println("BLE:");
    Serial.println("ble_start");
    Serial.println("ble_stop");
    Serial.println();
    Serial.println("MQTT:");
    Serial.println("mqtt_on");
    Serial.println("mqtt_on <user> <password>");
    Serial.println("mqtt_off");
    Serial.println();
    Serial.println("Device Name:");
    Serial.println("name <new device name>");
    Serial.println("===================================================");
    Serial.println();
}



void printFullStatus()
{
    Serial.println();
    Serial.println("---------------- Current Status ----------------");
    printWiFiStatus(eqsp32.getWiFiStatus());
    printBleStatus(eqsp32.getBleStatus());
    printMQTTStatus(eqsp32.getMQTTStatus());

    Serial.print("Device Online: ");
    Serial.println(eqsp32.isDeviceOnline() ? "Yes" : "No");

    Serial.print("Local IP: ");
    Serial.println(eqsp32.localIP());
    Serial.println("------------------------------------------------");
    Serial.println();
}


void printWiFiStatus(EQ_WifiStatus status)
{
    Serial.print("WiFi Status: ");
    switch (status)
    {
        case EQ_WF_DISCONNECTED:
            Serial.println("Disconnected");
            break;

        case EQ_WF_CONNECTED:
            Serial.println("Connected");
            break;

        case EQ_WF_RECONNECTING:
            Serial.println("Reconnecting");
            break;

        case EQ_WF_SCANNING:
            Serial.println("Scanning");
            break;

        default:
            Serial.println("Unknown");
            break;
    }
}

void printBleStatus(EQ_BleStatus status)
{
    Serial.print("BLE Status: ");
    switch (status)
    {
        case EQ_BLE_DISCONNECTED:
            Serial.println("Initialized but not connected and not advertising");
            break;

        case EQ_BLE_CONNECTED:
            Serial.println("Client connected");
            break;

        case EQ_BLE_ADVERTISING:
            Serial.println("Advertising");
            break;

        case EQ_BLE_SUBSCRIBED:
            Serial.println("Client subscribed");
            break;

        case EQ_BLE_NO_INIT:
            Serial.println("Not initialized or not managed by EQSP32");
            break;

        default:
            Serial.println("Unknown");
            break;
    }
}

void printMQTTStatus(EQ_MQTTBrokerStatus status)
{
    Serial.print("MQTT Status: ");
    switch (status)
    {
        case EQ_MQTT_DISCONNECTED:
            Serial.println("Disconnected");
            break;

        case EQ_MQTT_CONNECTED:
            Serial.println("Connected");
            break;

        case EQ_MQTT_CONNECTING:
            Serial.println("Connecting");
            break;

        case EQ_MQTT_NO_INIT:
            Serial.println("Not initialized");
            break;

        default:
            Serial.println("Unknown");
            break;
    }
}


String trimCopy(const String& s)
{
    String out = s;
    out.trim();
    return out;
}

String getFirstToken(String& line)
{
    line.trim();
    int spaceIndex = line.indexOf(' ');
    if (spaceIndex < 0)
    {
        String token = line;
        line = "";
        return token;
    }

    String token = line.substring(0, spaceIndex);
    line = line.substring(spaceIndex + 1);
    line.trim();
    return token;
}
