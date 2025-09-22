/**
 * @file EQSP32_ModbusTCP_Server_Demo.ino
 * @brief Demonstrates EQSP32 as a Modbus TCP Server (Slave).
 *
 * This example shows how to use the EQSP32 together with the
 * emelianov/modbus-esp8266 library to act as a Modbus TCP server.
 *
 * Features:
 * - EQSP32 provisions its network connection (Wi-Fi or Ethernet).
 * - Runs a Modbus TCP server listening on port 502 (default).
 * - Provides two holding registers:
 *    - REG_UPTIME  → updated every second with the server's uptime in seconds.
 *    - REG_COMMAND → writable by the client; detects and prints changes.
 * - Serial Monitor prints uptime and any new command values received.
 *
 * Hardware Setup:
 * - EQSP32 connected via Wi-Fi or Ethernet.
 * - A Modbus TCP client available on the same network (e.g., PLC, SCADA, or another EQSP32 running as a client).
 * - If another EQSP32 is used as the Modbus TCP client:
 *    • Both devices can be connected on the same LAN via a router (using DHCP or static IP).
 *    • Or connected directly with each other via a crossover Ethernet cable (must both have static IP).
 * - For direct connections without a DHCP server, configure **static IPs** on both EQSP32 devices in the same subnet.
 * - Ensure the Modbus TCP server IP (this device's IP) matches the server IP set in the client's side.
 *
 * Requirements:
 * - EQSP32 library for provisioning and network handling.
 * - emelianov/modbus-esp8266 library for Modbus TCP protocol.
 * - EQConnect mobile app to setup network credentials or static IP.
 *
 * Notes:
 * - In Modbus terms, the EQSP32 is the **server (slave)** and the remote device is the **client (master)**.
 * - Adjust IP settings if you need a fixed IP (default uses EQSP32 provisioning).
 *
 * @author Erqos Technologies
 * @date 2025-09-22
 */

#include <EQSP32.h>
#include <ModbusIP_ESP8266.h>

// ===================== GLOBAL OBJECTS =====================
EQSP32 eqsp32;             // EQSP32 system (handles Wi-Fi/Ethernet provisioning)
ModbusIP mb;               // Modbus-ESP8266 library object
EQTimer uptimeTimer(1000); // Timer for periodic uptime updates (1 second)

// ===================== REGISTER MAP =====================
const uint16_t REG_UPTIME  = 1;  // Holding register 1 (auto-updated uptime in seconds)
const uint16_t REG_COMMAND = 2;  // Holding register 2 (writable by client)

// ===================== VARIABLES =====================
uint16_t lastCommand = 0;  // Track the last command value to detect changes

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Starting EQSP32 as Modbus TCP Server (Slave) ...");

  EQSP32Configs eqCfg;
  // ================== NETWORK SWITCHING BEHAVIOR ==================
  // Default behavior:
  // - EQSP32 automatically switches between Wi-Fi and Ethernet.
  // - Ethernet has priority → if both are connected, Ethernet will be used.
  // - Only one interface is active at a time.
  //
  // When disableNetSwitching = true:
  // - Both Wi-Fi and Ethernet can remain active simultaneously.
  // - Use case example:
  //    • Wi-Fi → keep EQSP32 online via DHCP (internet, MQTT, OTA, cloud).
  //    • Ethernet → handle a separate local connection (e.g., Modbus TCP with PLC).
  //
  // ⚠️ Important note:
  // - When disableNetSwitching is set to true, any static IP configuration
  //   applies ONLY to Ethernet.
  // - Wi-Fi will always use DHCP in this mode.
  //
  eqCfg.disableNetSwitching = false;

  // Start EQSP32 (handles provisioning via EQConnect app)
  eqsp32.begin(eqCfg, true); // true = verbose logging from EQSP32
  

  // Wait until device is connected on router or directly to another device via Ethernet
  while (!eqsp32.isDeviceOnline()) {
    Serial.println("❌ EQSP32 not connected. Waiting WiFi or Ethernet connection ...");
    delay(2000);
  }

  // Network ready
  Serial.println("✅ EQSP32 connected!");
  Serial.print("🌐 Local IP: ");
  Serial.println(eqsp32.localIP());

  // Start Modbus as server (slave)
  mb.server();

  // Add holding registers
  mb.addHreg(REG_UPTIME, 0);   // uptime register
  mb.addHreg(REG_COMMAND, 0);  // command register

  // Start uptime timer
  uptimeTimer.start();
}

// ===================== LOOP =====================
void loop() {
  // Handle incoming Modbus TCP requests
  mb.task();

  // -------- UPDATE UPTIME --------
  if (uptimeTimer.isExpired()) {
    mb.Hreg(REG_UPTIME, millis() / 1000);

    Serial.println("\n================== POLL CYCLE ==================");
    Serial.printf("⏱️ [SERVER] Uptime: %d sec | Current Command: %d\n",
                  mb.Hreg(REG_UPTIME),
                  mb.Hreg(REG_COMMAND));

    Serial.println("");
    uptimeTimer.reset();
  }

  // -------- DETECT NEW COMMAND --------
  uint16_t currentCommand = mb.Hreg(REG_COMMAND);
  if (currentCommand != lastCommand) {
    Serial.printf("🆕 [SERVER] New command received: %d (previous: %d)\n",
                  currentCommand, lastCommand);
    lastCommand = currentCommand;
  }

  delay(10); // FreeRTOS friendly
}
