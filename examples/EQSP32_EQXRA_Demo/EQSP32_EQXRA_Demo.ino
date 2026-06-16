/**
 * @file EQSP32_EQXRA_Demo.ino
 * @brief Demonstrates EQXRA relay outputs, analog outputs, feedback monitoring, and optional watchdog/sync-output configurations.
 *
 * This demo initializes the EQSP32, checks for an EQXRA expansion module, configures
 * two relay outputs and two 0-10 V analog outputs, then periodically toggles the
 * relays while swapping the analog output targets.
 *
 * Analog outputs use asynchronous AOUT mode by default. Uncomment
 * `DEMO_USE_SYNC_AOUT` to use synchronized SAOUT mode, where AO values are staged
 * and applied together using `EQXRA_SAOUT_APPLY()`.
 *
 * Optional relay and analog-output watchdog protection can be enabled by uncommenting
 * their configuration macros. Analog output feedback deviation monitoring is enabled
 * by default with a 250 mV threshold and 500 ms trigger delay.
 *
 * The sketch periodically prints relay state, AO target/feedback values, relay supply
 * voltage, module status, and analog output status. EQConnect / Erqos IoT connectivity
 * is handled internally by `eqsp32.begin()`; no Wi-Fi, BLE, MQTT, or cloud setup code
 * is required.
 *
 * Requirements:
 * - EQSP32 controller with EQSP32 library installed.
 * - EQXRA expansion module connected to the EQSP32 expansion port.
 * - Arduino IDE: ESP32-S3, 8 MB flash, 8M with SPIFFS, ESP32 Arduino core v2.0.17.
 * - Serial Monitor: 115200 baud.
 *
 * @author Erqos Technologies
 * @date 2026-06-15
 */


#include <EQSP32.h>


// -------------------- Configuration --------------------
#define EQXRA_INDEX                 1

// #define DEMO_USE_SYNC_AOUT        // Uncomment for synchronized SAOUT mode; leave commented for asynchronous AOUT mode.
// #define ENABLE_RELAY_WATCHDOG     // Uncomment to enable relay watchdog protection.
// #define ENABLE_AO_WATCHDOG        // Uncomment to enable analog output watchdog protection.
// #define CONFIG_AO_DEVIATION_GUARD // Enabled by default: AO feedback deviation guard, 250 mV threshold, 500 ms delay.

// Demo timing
#define COMMAND_REFRESH_MS          250
#define STATE_CHANGE_MS             3000
#define SERIAL_PRINT_MS             1000
#define LOOP_DELAY_MS               10

// Watchdog/deviation settings
#define RELAY_WDG_TIMEOUT_MS        200
#define AO1_WDG_TIMEOUT_MS          1000
#define AO2_WDG_TIMEOUT_MS          1000
#define AO_DEV_THRESHOLD_MV         350
#define AO_DEV_DELAY_MS             100


// -------------------- EQXRA Channel Mapping --------------------
#define EQXRA_MODULE                EQXRA(EQXRA_INDEX)

#define EQXRA_RELAY_1_PIN           EQXRA(EQXRA_INDEX, EQXRA_RL_1)
#define EQXRA_RELAY_2_PIN           EQXRA(EQXRA_INDEX, EQXRA_RL_2)

#define EQXRA_AO1_PIN               EQXRA(EQXRA_INDEX, EQXRA_AO_1)
#define EQXRA_AO2_PIN               EQXRA(EQXRA_INDEX, EQXRA_AO_2)

#define EQXRA_AO1_FB_PIN            EQXRA(EQXRA_INDEX, EQXRA_AOFB_1)
#define EQXRA_AO2_FB_PIN            EQXRA(EQXRA_INDEX, EQXRA_AOFB_2)
#define EQXRA_RL_SUPPLY_PIN         EQXRA(EQXRA_INDEX, EQXRA_RL_SUPPLY)
#define EQXRA_MODULE_STATUS_PIN     EQXRA(EQXRA_INDEX, EQXRA_SYSSTAT)
#define EQXRA_AOUT_STATUS_PIN       EQXRA(EQXRA_INDEX, EQXRA_AOSTAT)

// -------------------- Globals --------------------
EQSP32 eqsp32;

EQTimer commandRefreshTimer(COMMAND_REFRESH_MS);
EQTimer stateChangeTimer(STATE_CHANGE_MS);
EQTimer serialPrintTimer(SERIAL_PRINT_MS);

bool eqxraReady = false;
bool relayState = false;

int ao1Target_mV = 2500;
int ao2Target_mV = 7500;

// -------------------- Helpers --------------------
void configureEQXRA();
void updateDemoState();
void writeEQXRAOutputs();
void printEQXRAStatus();

void printModuleStatus(uint8_t status);
void printAoutChannelStatus(const char* label, uint8_t status);
uint8_t getAO1Status(uint8_t status);
uint8_t getAO2Status(uint8_t status);

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("\nStarting EQSP32 EQXRA Options Demo...");

  eqsp32.begin();

  eqxraReady = eqsp32.isModuleDetected(EQXRA_MODULE);
  if (!eqxraReady) {
    Serial.println("EQXRA module not detected. Check expansion connection and power-cycle the system.");
    return;
  }

  configureEQXRA();

  commandRefreshTimer.start();
  stateChangeTimer.start();
  serialPrintTimer.start();

  writeEQXRAOutputs();
}

// -------------------- Loop --------------------
void loop() {
  // -------------------- Read Inputs / Feedback --------------------
  if (!eqxraReady) {
    delay(LOOP_DELAY_MS);
    return;
  }

  // -------------------- Process Logic --------------------
  if (stateChangeTimer.isExpired()) {
    stateChangeTimer.reset();
    updateDemoState();
  }

  // -------------------- Update Outputs --------------------
  if (commandRefreshTimer.isExpired()) {
    commandRefreshTimer.reset();
    writeEQXRAOutputs();
  }

  // -------------------- Diagnostics --------------------
  if (serialPrintTimer.isExpired()) {
    serialPrintTimer.reset();
    printEQXRAStatus();
  }

  delay(LOOP_DELAY_MS);
}

// -------------------- EQXRA Configuration --------------------
void configureEQXRA() {
  Serial.println("Configuring EQXRA...");

  // Relay outputs
  eqsp32.pinMode(EQXRA_RELAY_1_PIN, RELAY);
  eqsp32.pinMode(EQXRA_RELAY_2_PIN, RELAY);

  // Analog outputs
#ifdef DEMO_USE_SYNC_AOUT
  eqsp32.pinMode(EQXRA_AO1_PIN, SAOUT);
  eqsp32.pinMode(EQXRA_AO2_PIN, SAOUT);
  Serial.println("AOUT mode: SYNC / SAOUT");
#else
  eqsp32.pinMode(EQXRA_AO1_PIN, AOUT);
  eqsp32.pinMode(EQXRA_AO2_PIN, AOUT);
  Serial.println("AOUT mode: ASYNC / AOUT");
#endif

  // ----------------------------------------------------------------
  // Relay watchdog option
  // ----------------------------------------------------------------
#ifdef ENABLE_RELAY_WATCHDOG
  eqsp32.configWDG(EQXRA_RELAY_1_PIN, RELAY_WDG_TIMEOUT_MS);
#endif

  // ----------------------------------------------------------------
  // Analog output watchdog option
  // ----------------------------------------------------------------
#ifdef ENABLE_AO_WATCHDOG
  eqsp32.configWDG(EQXRA_AO1_PIN, AO1_WDG_TIMEOUT_MS);
  eqsp32.configWDG(EQXRA_AO2_PIN, AO2_WDG_TIMEOUT_MS);
#endif

  // ----------------------------------------------------------------
  // Analog feedback deviation option
  // ----------------------------------------------------------------
#ifdef CONFIG_AO_DEVIATION_GUARD
  eqsp32.configFBDEV(EQXRA_AO1_PIN, AO_DEV_THRESHOLD_MV, AO_DEV_DELAY_MS);
  eqsp32.configFBDEV(EQXRA_AO2_PIN, AO_DEV_THRESHOLD_MV, AO_DEV_DELAY_MS);
#endif

  Serial.println("EQXRA configuration complete.");
}

// -------------------- Demo State --------------------
void updateDemoState() {
  relayState = !relayState;

  if (relayState) {
    ao1Target_mV = 2500;
    ao2Target_mV = 7500;
  } else {
    ao1Target_mV = 7500;
    ao2Target_mV = 2500;
  }

  Serial.println("\nDemo state changed.");
}

// -------------------- Output Update --------------------
void writeEQXRAOutputs() {
  int relayValue = relayState ? 1000 : 0;

  eqsp32.pinValue(EQXRA_RELAY_1_PIN, relayValue);
  eqsp32.pinValue(EQXRA_RELAY_2_PIN, relayValue);

#ifdef DEMO_USE_SYNC_AOUT
  // Stage AO1. Output voltage will not change until APPLY.
  eqsp32.pinValue(EQXRA_AO1_PIN, ao1Target_mV);

  // Stage AO2 and apply both SAOUT channels together.
  eqsp32.pinValue(EQXRA_AO2_PIN, EQXRA_SAOUT_APPLY(ao2Target_mV));
#else
  // Async mode: each AO updates immediately.
  eqsp32.pinValue(EQXRA_AO1_PIN, ao1Target_mV);
  eqsp32.pinValue(EQXRA_AO2_PIN, ao2Target_mV);
#endif
}

// -------------------- Status Print --------------------
void printEQXRAStatus() {
  int ao1Feedback_mV = eqsp32.readPin(EQXRA_AO1_FB_PIN);
  int ao2Feedback_mV = eqsp32.readPin(EQXRA_AO2_FB_PIN);
  int relaySupply_mV = eqsp32.readPin(EQXRA_RL_SUPPLY_PIN);

  uint8_t moduleStatus = (uint8_t)eqsp32.readPin(EQXRA_MODULE_STATUS_PIN);
  uint8_t aoutStatus = (uint8_t)eqsp32.readPin(EQXRA_AOUT_STATUS_PIN);

  Serial.println("\n---------------- EQXRA Status ----------------");

  Serial.print("Relay state: ");
  Serial.println(relayState ? "ON" : "OFF");

  Serial.print("AO1 target / feedback: ");
  Serial.print(ao1Target_mV);
  Serial.print(" mV / ");
  Serial.print(ao1Feedback_mV);
  Serial.println(" mV");

  Serial.print("AO2 target / feedback: ");
  Serial.print(ao2Target_mV);
  Serial.print(" mV / ");
  Serial.print(ao2Feedback_mV);
  Serial.println(" mV");

  Serial.print("Relay supply: ");
  Serial.print(relaySupply_mV);
  Serial.println(" mV");

  Serial.print("Module status: ");
  printModuleStatus(moduleStatus);
  Serial.println();

  Serial.print("AO1 status: ");
  printAoutChannelStatus("AO1", getAO1Status(aoutStatus));
  Serial.println();

  Serial.print("AO2 status: ");
  printAoutChannelStatus("AO2", getAO2Status(aoutStatus));
  Serial.println();
}

// -------------------- Status Decode Helpers --------------------
void printModuleStatus(uint8_t status) {
  if (status == EQXRA_SYS_STATUS_OK) {
    Serial.print("OK");
    return;
  }

  if (status & EQXRA_SYS_STATUS_RL_WDG_TRIP)   Serial.print("RL_WDG_TRIP ");
  if (status & EQXRA_SYS_STATUS_AO_WDG_TRIP)   Serial.print("AO_WDG_TRIP ");
  if (status & EQXRA_SYS_STATUS_SUPPLY_UV)     Serial.print("SUPPLY_UV ");
  if (status & EQXRA_SYS_STATUS_SUPPLY_OV)     Serial.print("SUPPLY_OV ");
  if (status & EQXRA_SYS_STATUS_DAC_ERR)       Serial.print("DAC_ERR ");
  if (status & EQXRA_SYS_STATUS_AOUT_FAULT)    Serial.print("AOUT_FAULT ");
  if (status & EQXRA_SYS_STATUS_GENERAL_FAULT) Serial.print("GENERAL_FAULT ");
}

void printAoutChannelStatus(const char* label, uint8_t status) {
  if (status == EQXRA_AOUT_STATUS_OK) {
    Serial.print("OK");
    return;
  }

  if (status == EQXRA_AOUT_STATUS_CH_NA) {
    Serial.print(label);
    Serial.print("_NA");
    return;
  }

  if (status & EQXRA_AOUT_STATUS_CH_DEV) Serial.print("DEV ");
  if (status & EQXRA_AOUT_STATUS_CH_OV)  Serial.print("OV ");
  if (status & EQXRA_AOUT_STATUS_CH_SC)  Serial.print("SC ");
}

uint8_t getAO1Status(uint8_t status) {
  return (status & EQXRA_AOUT_STATUS_AO1_MASK) >> EQXRA_AOUT_STATUS_AO1_SHIFT;
}

uint8_t getAO2Status(uint8_t status) {
  return (status & EQXRA_AOUT_STATUS_AO2_MASK) >> EQXRA_AOUT_STATUS_AO2_SHIFT;
}