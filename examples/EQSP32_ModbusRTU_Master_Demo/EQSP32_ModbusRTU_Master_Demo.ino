/**
 * @file EQSP32_ModbusRTU_Master_Demo.ino
 * @brief Modbus RTU master example using EQSP32's RS485 interface.
 *
 * This example:
 * - Uses `getPin()` to retrieve RS485_TX, RS485_RX, and RS485_EN pin mappings.
 * - Initializes Serial2 for Modbus RTU master operation.
 * - Sends periodic requests to a Modbus slave to read holding register 10.
 * - Handles RS485 direction control with pre/post transmission callbacks.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS485 interface (no external transceiver needed).
 * - USB-RS485 adapter connected to a PC running Modbus client software,
 *   or another EQSP32 running a Modbus RTU slave demo.
 *
 * Wiring:
 * - EQSP32 RS485 A/TX  -> RS485 A of slave or adapter
 * - EQSP32 RS485 B/RX  -> RS485 B of slave or adapter
 *
 * @date 2025-05-15
 */

#include <EQSP32.h>
#include <ModbusMaster.h>

// === Constants ===
#define MODBUS_SLAVE_ID     1
#define MODBUS_REG_ADDR     10
#define MODBUS_REG_COUNT    1
#define MODBUS_POLL_DELAY   1000

// === Objects ===
EQSP32 eqsp32;
ModbusMaster node;

int txPin, rxPin, enPin;

// === RS485 Direction Control ===
void modbusEnableTx() {
  digitalWrite(enPin, HIGH);  // RS485 to transmit
}

void modbusEnableRx() {
  digitalWrite(enPin, LOW);   // RS485 to receive
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n🔁 Starting EQSP32 Modbus RTU Master Demo...");

  EQSP32Configs _myEQConfigs;
  _myEQConfigs.disableErqosIoT = true;
  eqsp32.begin(_myEQConfigs);

  // Retrieve EQSP32 RS485 hardware pins
  txPin = eqsp32.getPin(EQ_RS485_TX);
  rxPin = eqsp32.getPin(EQ_RS485_RX);
  enPin = eqsp32.getPin(EQ_RS485_EN);

  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);  // Default to RX mode

  // Setup RS485 communication
  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);

  // Setup Modbus Master
  node.begin(MODBUS_SLAVE_ID, Serial2);
  node.preTransmission(modbusEnableTx);
  node.postTransmission(modbusEnableRx);
}

void loop() {
  uint8_t result = node.readHoldingRegisters(MODBUS_REG_ADDR, MODBUS_REG_COUNT);
  if (result == node.ku8MBSuccess) {
    uint16_t value = node.getResponseBuffer(0);
    Serial.print("✅ Register ");
    Serial.print(MODBUS_REG_ADDR);
    Serial.print(": ");
    Serial.println(value);
  } else {
    Serial.print("❌ Modbus Read Error: ");
    Serial.println(result, HEX);
  }

  delay(MODBUS_POLL_DELAY);
}
