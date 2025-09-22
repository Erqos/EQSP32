/**
 * @file EQSP32_ModbusTCP_Client_Demo.ino
 * @brief Demonstrates EQSP32 as a Modbus TCP Client (Master).
 *
 * This example shows how to use the EQSP32 together with the
 * emelianov/modbus-esp8266 library to act as a Modbus TCP client.
 *
 * Features:
 * - EQSP32 provisions its network connection (Wi-Fi or Ethernet).
 * - Connects as a Modbus TCP client to a fixed server IP and port.
 * - Periodically (every 5 seconds):
 *    - Reads the server’s uptime from holding register 1 (REG_UPTIME).
 *    - Writes a local counter value to holding register 2 (REG_COUNTER).
 * - Serial Monitor prints all client → server requests and server → client responses.
 *
 * Hardware Setup:
 * - EQSP32 connected via Wi-Fi or Ethernet.
 * - A Modbus TCP server available on the same network (e.g., PLC, SCADA, or another EQSP32 running as a Modbus server).
 * - If another EQSP32 is used as the Modbus TCP server:
 *    • Both devices can be connected on the same LAN via a router (using DHCP or static IP).
 *    • Or connected directly with each other via a crossover Ethernet cable (must both have static IP).
 * - For direct connections without a DHCP server, configure **static IPs** on both EQSP32 devices in the same subnet.
 * - Ensure the Modbus TCP server IP matches the `serverIP` set in this sketch.
 *
 * Requirements:
 * - EQSP32 library for provisioning and network handling.
 * - emelianov/modbus-esp8266 library for Modbus TCP protocol.
 * - EQConnect mobile app to setup network credentials or static IP.
 *
 * Notes:
 * - In Modbus terms, the EQSP32 is the **client (master)** and the remote device is the **server (slave)**.
 * - Adjust the `serverIP` to match your server’s IP address.
 * - The server must be configured to have registers 1 and 2 available for reading/writing.
 *
 * @author Erqos Technologies
 * @date 2025-09-22
 */


#include <EQSP32.h>
#include <ModbusIP_ESP8266.h>

// ===================== GLOBAL OBJECTS =====================
EQSP32 eqsp32;      // EQSP32 system (Also handles WiFi/Ethernet provisioning)
ModbusIP mb;        // Modbus-ESP8266 library object
EQTimer pollTimer(5000);  // Timer for periodic polling (5 seconds)

// ===================== SERVER SETTINGS =====================
IPAddress serverIP(192, 168, 1, 100);  // The remote Modbus server IP to read from and write to (adjust as needed)
const int serverPort = 502;            // Modbus TCP port (default = 502)

// ===================== REGISTER MAP (server side) =====================
const uint16_t REG_UPTIME  = 1;   // Server uptime register (to read the server's up time)
const uint16_t REG_COUNTER = 2;   // Server counter register (to write a counter to the server)

// ===================== VARIABLES =====================
uint16_t counter   = 0;  // Local counter (to send to server)
uint16_t uptimeVal = 0;  // Buffer for server uptime (to store read value from server)

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Starting EQSP32 as Modbus TCP Client (Master) ...");

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

  // Start Modbus as client
  mb.client();

  // Try connecting to server
  Serial.print("🔗 Attempting connection to Modbus Server at ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);

  if (!mb.connect(serverIP, serverPort)) {
    Serial.println("❌ Initial connection to Modbus server FAILED!");
  } else {
    Serial.println("✅ Connected to Modbus server.");
  }

  pollTimer.start(); // Start periodic polling
}

// ===================== LOOP =====================
void loop() {
  // Poll remote registers every 5 seconds
  if (pollTimer.isExpired()) {
    Serial.println("\n================== POLL CYCLE ==================");
    if (mb.isConnected(serverIP)) {
      // -------- READ SERVER UPTIME --------
      Serial.printf("📥 [CLIENT → SERVER] Requesting REG_UPTIME [%d]...\n", REG_UPTIME);
      if (mb.readHreg(serverIP, REG_UPTIME, &uptimeVal, 1)) {
        Serial.printf("📡 [SERVER → CLIENT] Response: Uptime = %d sec\n", uptimeVal);
      } else {
        Serial.println("⚠️ Failed to read REG_UPTIME from server.");
      }

      // -------- WRITE COUNTER TO SERVER --------
      Serial.printf("📤 [CLIENT → SERVER] Writing counter value (%d) to REG_COUNTER [%d]...\n", counter, REG_COUNTER);
      if (mb.writeHreg(serverIP, REG_COUNTER, counter)) {
        Serial.println("✅ Counter successfully written to server.");
      } else {
        Serial.println("⚠️ Failed to write REG_COUNTER to server.");
      }

      counter++;  // Increment local counter for next cycle
    } else {
      // Server disconnected → retry
      Serial.println("⚠️ Connection lost! Retrying...");
      mb.connect(serverIP, serverPort);
    }
    Serial.println("");

    pollTimer.reset(); // Restart poll timer
  }

  // Handle Modbus TCP communication (async)
  mb.task();

  delay(10); // FreeRTOS friendly
}
