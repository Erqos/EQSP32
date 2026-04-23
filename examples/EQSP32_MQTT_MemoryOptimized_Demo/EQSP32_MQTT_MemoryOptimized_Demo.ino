/**
 * @file EQSP32_MQTT_MemoryOptimized_Demo.ino
 * @brief Demonstrates bulk MQTT entity creation using configuration arrays, including text control entities,
 *        and shows how to publish selected control and display entity states with and without MQTT retain.
 *
 * This example assumes:
 *   - Erqos IoT functionality is enabled.
 *   - MQTT interfacing is managed by the EQSP32 library.
 *   - The created entities are intended for use through MQTT-based interfaces such as Home Assistant
 *     or other MQTT clients.
 *
 * Bulk Entity Creation:
 *   - Uses `createControl_Switches()` to create multiple switch control entities at once.
 *   - Uses `createControl_Values()` to create multiple value control entities at once.
 *   - Uses `createControl_Texts()` to create multiple text control entities at once.
 *   - Uses `createDisplay_BinarySensors()` to create multiple binary sensor display entities at once.
 *   - Uses `createDisplay_Sensors()` to create multiple sensor display entities at once.
 *
 * Text Entity Support:
 *   - Demonstrates the `TextEntityConfig` structure for creating text input entities.
 *   - Shows how text control values can be read at runtime using `readControl_Text()`.
 *
 * Retained Publish Demonstration:
 *   - Demonstrates the `retain` parameter in the update functions for control and display entities.
 *   - Control entity updates may use:
 *       `updateControl_Switch(..., value, retain)`
 *       `updateControl_Value(..., value, retain)`
 *       `updateControl_Text(..., value, retain)`
 *   - Display entity updates may use:
 *       `updateDisplay_BinarySensor(..., value, retain)`
 *       `updateDisplay_Sensor(..., value, retain)`
 *   - Shows retained publishing by calling selected update functions with `retain = true`.
 *   - Shows non-retained publishing by calling selected update functions with `retain = false`.
 *
 * Runtime Behavior:
 *   - Always reads the current control values.
 *   - Prints control value changes only when a new value is detected.
 *   - Updates display entities periodically using the standard display update functions.
 *   - Demonstrates simple logic by reflecting control values into display entities.
 *   - Monitors and prints MQTT broker status changes using `getMQTTStatus()`.
 *   - If `mqttAutoUpdateStateFromCommand` is disabled, the application manually republishes all control states
 *     every 5 seconds.
 *
 * Optional Configuration:
 *   - `disableSystemMQTTEntities` may be enabled in `EQSP32Configs` to prevent automatic creation
 *     and update of the default system MQTT entities.
 *   - `mqttAutoUpdateStateFromCommand` may be set to `false` to disable automatic state updates
 *     from incoming command topics.
 *   - When `mqttAutoUpdateStateFromCommand` is disabled, the application must manually keep the
 *     corresponding control state topics synchronized by calling the appropriate `updateControl_*()`
 *     functions for all relevant control entities.
 *   - `mqttBrokerIp` and `mqttBrokerPort` may be optionally overridden in `EQSP32Configs`
 *     to customize the MQTT broker connection used by the example.
 *
 * Notes:
 *   - This example shows the newer bulk entity creation approach, which is more compact and memory efficient than
 *     creating each entity individually.
 *   - Runtime read and update behavior remains the same as with the previous single-entity API style.
 *   - The `retain` parameter controls whether the current MQTT publish operation is retained by the broker.
 *   - Automatic command-to-state updates use normal non-retained state behavior.
 *   - If retained state publishing is required, the application must explicitly call the corresponding
 *     `updateControl_*()` function with `retain = true`, regardless of the automatic update setting.
 *
 * @author Erqos Technologies
 * @date 2026-04-23
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
EQSP32Configs myConfigs;        // Global so the configuration remains accessible after setup
EQTimer statusTimer(1000);
EQTimer manualControlUpdateTimer(5000);   // Used to manually republish control states every 5 s when auto command-to-state update is disabled

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

static float setpoint = 25.0f;
static float brightness = 0.0f;

static String deviceLabel = "EQSP32 Demo";
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
    Serial.println("Demonstrates bulk MQTT entity creation, text control entities,");
    Serial.println("and retained MQTT publishing.");
    Serial.println();

    // Optional:
    // Uncomment any of the settings below to customize MQTT behavior for this example.
    // myConfigs.disableSystemMQTTEntities = true;         // Disable automatic system MQTT entities
    // myConfigs.mqttAutoUpdateStateFromCommand = false;   // Disable automatic state update from incoming command topics
    // myConfigs.mqttBrokerIp = "homeassistant.local";     // Override broker IP / hostname
    // myConfigs.mqttBrokerPort = 1883;                    // Override broker port

    eqsp32.begin(myConfigs);

    prevMQTTStatus = eqsp32.getMQTTStatus();
    printMQTTStatus(prevMQTTStatus);

    createDemoEntities();
    
    statusTimer.start();
    manualControlUpdateTimer.start();
}

void loop()
{
    readControls();        // Always read current control values
    processLogic();        // Apply logic and manual control-state publishing if needed
    updateDisplays();      // Periodic display updates
    printChanges();        // Print only when values changed
    monitorMQTTStatus();   // Print only when MQTT status changed

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

        // Clear the reset request after processing it.
        // This update is always done because it is part of the demo logic itself.
        updateControl_Switch("Reset Counter", false, true);
        resetCounter = false;
    }

    outputActive = outputEnabled;
    currentValue = setpoint;

    // If automatic state update from command is disabled,
    // manually republish all control states every 5 seconds.
    //
    // Important:
    // - Automatic command-to-state updates use normal non-retained state behavior.
    // - If retained state publishing is needed, the application must explicitly call
    //   updateControl_*() with retain = true, regardless of the auto-update setting.
    if (!myConfigs.mqttAutoUpdateStateFromCommand && manualControlUpdateTimer.isExpired())
    {
        manualControlUpdateTimer.reset();

        updateControl_Switch("Enable Output", outputEnabled, true);
        updateControl_Switch("Reset Counter", resetCounter, true);

        updateControl_Value("Setpoint", setpoint, true);
        updateControl_Value("Brightness", brightness, false);

        updateControl_Text("Device Label", deviceLabel, true);
        updateControl_Text("Operator Note", operatorNote, false);

        Serial.println("Manual control state update published.");
    }
}

void updateDisplays()
{
    if (statusTimer.isExpired())
    {
        loopCounter++;
        statusTimer.reset();

        // Retained display update examples
        updateDisplay_BinarySensor("Output Active", outputActive, true);
        updateDisplay_Sensor("Current Value", currentValue, true);

        // Non-retained display update examples
        updateDisplay_BinarySensor("Online", eqsp32.isDeviceOnline());
        updateDisplay_Sensor("Loop Counter", (float)loopCounter);
        updateDisplay_Sensor("Free Heap", (float)ESP.getFreeHeap());
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