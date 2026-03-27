/**
 * @file EQSP32_MQTT_MemoryOptimized_Demo.ino
 * @brief Demonstrates bulk MQTT entity creation using configuration arrays, including the new text control entity type.
 *
 * This example assumes:
 *   - Erqos IoT functionality is enabled.
 *   - MQTT interfacing is managed by the EQSP32 library.
 *   - The created entities are intended for use through MQTT-based interfaces such as Home Assistant or other MQTT clients.
 *
 * Bulk Entity Creation:
 *   - Uses `createControl_Switches()` to create multiple switch control entities at once.
 *   - Uses `createControl_Values()` to create multiple value control entities at once.
 *   - Uses `createControl_Texts()` to create multiple text control entities at once.
 *   - Uses `createDisplay_BinarySensors()` to create multiple binary sensor display entities at once.
 *   - Uses `createDisplay_Sensors()` to create multiple sensor display entities at once.
 *
 * Text Entity Support:
 *   - Demonstrates the new `TextEntityConfig` structure for creating text input entities.
 *   - Shows how text control values can be read at runtime using `readControl_Text()`.
 *
 * Runtime Behavior:
 *   - Reads control entities using the same read functions as the previous implementation style.
 *   - Updates display entities using the same update functions as the previous implementation style.
 *   - Mirrors switch, value, and text control changes to the USB serial monitor for easy monitoring.
 *   - Demonstrates simple logic by reflecting control values into display entities.
 *   - Monitors and prints MQTT broker status changes using `getMQTTStatus()`.
 *
 * Optional Configuration:
 *   - `disableSystemMQTTEntities` may be enabled in `EQSP32Configs` to prevent automatic creation
 *     and update of the default system MQTT entities.
 *   - `mqttBrokerIp`, `mqttBrokerPort`, and `mqtt_broker_ca` may be optionally overridden in
 *     `EQSP32Configs` to customize the MQTT broker connection used by the example.
 *
 * Notes:
 *   - This example shows the newer bulk entity creation approach, which is more compact than
 *     creating each entity individually.
 *   - Runtime read and update behavior remains the same as with the previous single-entity API style.
 *
 * @author Erqos Technologies
 * @date 2026-03-24
 */


#include <EQSP32.h>

void createDemoEntities();
void readControls();
void processLogic();
void updateDisplays();
void printChanges();
void printMQTTStatus(EQ_MQTTBrokerStatus status);
void monitorMQTTStatus();

EQSP32 eqsp32;
EQTimer statusTimer(1000);

// -----------------------------------------------------------------------------
// Bulk entity configuration
// -----------------------------------------------------------------------------
static const SwitchEntityConfig controlSwitches[] = {
    {"Enable Output", "mdi:toggle-switch"},
    {"Reset Counter", "mdi:restart"}
};

static const ValueEntityConfig controlValues[] = {
    {"Setpoint",   0, 100, 1, "mdi:tune"},
    {"Brightness", 0, 100, 0, "mdi:brightness-6"}
};

static const TextEntityConfig controlTexts[] = {
    {"Device Label", 0, 24, false, "", "mdi:tag-text"},
    {"Operator Note", 0, 32, false, "", "mdi:text"}
};

static const BinarySensorEntityConfig displayBinarySensors[] = {
    {"Output Active", "mdi:flash", "power"},
    {"Online", "mdi:wifi", "connectivity"}
};

static const SensorEntityConfig displaySensors[] = {
    {"Current Value", 1, "%", "mdi:gauge", ""},
    {"Loop Counter",  0, "cnt", "mdi:counter", ""},
    {"Free Heap",     0, "B", "mdi:memory", ""}
};

// -----------------------------------------------------------------------------
// Runtime variables
// -----------------------------------------------------------------------------
static bool outputEnabled = false;
static bool resetCounter = false;

static float setpoint = 0.0f;
static float brightness = 0.0f;

static String deviceLabel = "";
static String operatorNote = "";

static bool outputActive = false;
static float currentValue = 0.0f;
static uint32_t loopCounter = 0;

static bool prevOutputEnabled = false;
static bool prevResetCounter = false;
static float prevSetpoint = -999999.0f;
static float prevBrightness = -999999.0f;
static String prevDeviceLabel = "";
static String prevOperatorNote = "";

static EQ_MQTTBrokerStatus prevMQTTStatus = EQ_MQTT_NO_INIT;

void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println("EQSP32 Memory Optimized Entity Creation Demo");
    Serial.println("--------------------------------");
    Serial.println("Demonstrates bulk MQTT entity creation and text control entities stored in flash.");
    Serial.println();

    EQSP32Configs myConfigs;

    // Optional:
    // Uncomment any of the settings below to customize MQTT behavior for this example.
    // myConfigs.disableSystemMQTTEntities = true;      // Disable automatic system MQTT entities
    // myConfigs.mqttBrokerIp = "homeassistant.local";  // Override broker IP / hostname
    // myConfigs.mqttBrokerPort = 1883;                       // Override broker port
    // myConfigs.mqtt_broker_ca = "-----BEGIN CERTIFICATE-----\n"
    //                          "...\n"
    //                          "-----END CERTIFICATE-----\n";   // Optional TLS CA certificate
    eqsp32.begin(myConfigs);

    prevMQTTStatus = eqsp32.getMQTTStatus();
    printMQTTStatus(prevMQTTStatus);

    createDemoEntities();
    statusTimer.start();
}

void loop()
{
    readControls();
    processLogic();
    updateDisplays();
    printChanges();
    monitorMQTTStatus();

    delay(10);
}

void createDemoEntities()
{
    createControl_Switches(controlSwitches, sizeof(controlSwitches) / sizeof(controlSwitches[0]));
    createControl_Values(controlValues, sizeof(controlValues) / sizeof(controlValues[0]));
    createControl_Texts(controlTexts, sizeof(controlTexts) / sizeof(controlTexts[0]));
    createDisplay_BinarySensors(displayBinarySensors, sizeof(displayBinarySensors) / sizeof(displayBinarySensors[0]));
    createDisplay_Sensors(displaySensors, sizeof(displaySensors) / sizeof(displaySensors[0]));

    Serial.println("Bulk demo entities created.");
}

void readControls()
{
    outputEnabled = readControl_Switch("Enable Output");
    resetCounter = readControl_Switch("Reset Counter");

    setpoint = readControl_Value("Setpoint");
    brightness = readControl_Value("Brightness");

    deviceLabel = readControl_Text("Device Label");
    operatorNote = readControl_Text("Operator Note");
}

void processLogic()
{
    if (resetCounter)
    {
        loopCounter = 0;
        updateControl_Switch("Reset Counter", false);
        resetCounter = false;
    }

    outputActive = outputEnabled;
    currentValue = setpoint;
}

void updateDisplays()
{
    if (statusTimer.isExpired())
    {
        loopCounter++;
        statusTimer.reset();

        updateDisplay_Sensor("Free Heap", (float)ESP.getFreeHeap());

        updateDisplay_BinarySensor("Output Active", outputActive);
        updateDisplay_BinarySensor("Online", eqsp32.isDeviceOnline());

        updateDisplay_Sensor("Current Value", currentValue);
        updateDisplay_Sensor("Loop Counter", (float)loopCounter);
    }
}

void printChanges()
{
    if (outputEnabled != prevOutputEnabled)
    {
        Serial.print("Enable Output: ");
        Serial.println(outputEnabled ? "ON" : "OFF");
        prevOutputEnabled = outputEnabled;
    }

    if (resetCounter != prevResetCounter)
    {
        Serial.print("Reset Counter: ");
        Serial.println(resetCounter ? "ON" : "OFF");
        prevResetCounter = resetCounter;
    }

    if (setpoint != prevSetpoint)
    {
        Serial.print("Setpoint: ");
        Serial.println(setpoint);
        prevSetpoint = setpoint;
    }

    if (brightness != prevBrightness)
    {
        Serial.print("Brightness: ");
        Serial.println(brightness);
        prevBrightness = brightness;
    }

    if (deviceLabel != prevDeviceLabel)
    {
        Serial.print("Device Label: ");
        Serial.println(deviceLabel);
        prevDeviceLabel = deviceLabel;
    }

    if (operatorNote != prevOperatorNote)
    {
        Serial.print("Operator Note: ");
        Serial.println(operatorNote);
        prevOperatorNote = operatorNote;
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

void monitorMQTTStatus()
{
    EQ_MQTTBrokerStatus mqttStatus = eqsp32.getMQTTStatus();

    if (mqttStatus != prevMQTTStatus)
    {
        printMQTTStatus(mqttStatus);
        prevMQTTStatus = mqttStatus;
    }
}