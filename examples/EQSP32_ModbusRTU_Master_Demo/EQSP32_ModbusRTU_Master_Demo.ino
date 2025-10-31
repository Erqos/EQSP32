/**
 * @file EQSP32_ModbusRTU_Master_Demo.ino
 * @brief Non-blocking Modbus RTU master example using EQSP32's onboard RS485 interface.
 *
 * This example demonstrates how to implement an asynchronous Modbus RTU master
 * using the onboard RS485 transceiver of the EQSP32 and the `ModbusRTU` library
 * by Alexander Emelianov.
 *
 * Features:
 * - Uses `getPin()` to automatically retrieve RS485 TX, RX, and EN pin mappings.
 * - Implements a non-blocking Modbus RTU master using `ModbusRTU`.
 * - Continuously calls `mb.task()` in the main loop to process Modbus communication.
 * - Periodically reads Holding Register 10 from a Modbus slave every second.
 * - Automatically controls RS485 direction via the enable pin.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS485 interface (no external transceiver required).
 * - Connect to another EQSP32 running the Modbus RTU slave example,
 *   or a USB-RS485 adapter running Modbus simulator software.
 *
 * Wiring:
 *   EQSP32 RS485 A/TX  →  Slave RS485 A
 *   EQSP32 RS485 B/RX  →  Slave RS485 B
 *   GND                →  GND
 *
 * Notes:
 * - `getPin()` automatically initializes and assigns the RS485 UART pins.
 * - Communication runs asynchronously and non-blocking via `mb.task()`.
 * - Default Modbus configuration: 9600 bps, 8N1, Slave ID = 1, Holding Register = 10.
 * - Fully FreeRTOS-friendly with minimal CPU load.
 *
 * PC-Side Testing:
 * - Use Modbus software such as Simply Modbus Slave, or ModSim32.
 * - Configure as: RTU mode, Slave ID = 1, Baud = 9600, 8 data bits, no parity, 1 stop bit.
 * - Observe or modify Holding Register 10.
 *
 * @dependencies
 * - EQSP32 library (v1.0.4 or later)
 * - ModbusRTU library (by Alexander Emelianov)
 *
 * @date 2025-10-30
 * @author Erqos Technologies
 */

#include <Arduino.h>
#include <EQSP32.h>
#include <ModbusRTU.h>

// -------------------- Configuration --------------------
#define MODBUS_SLAVE_ID   1
#define MODBUS_REG_ADDR   10
#define MODBUS_REG_COUNT  1
#define MODBUS_POLL_MS    1000  // Poll every 1 second

// -------------------- Globals --------------------------
EQSP32 eqsp32;
ModbusRTU mb;
HardwareSerial& MB_SERIAL = Serial2;

int mbTx, mbRx, mbEn;
uint16_t modbusValue = 0;

EQTimer tModbusPoll(MODBUS_POLL_MS);   // Non-blocking poll timer

// -------------------- Modbus Callback -------------------
bool modbusReadCallback(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.printf("✅ Modbus HR[%u] = %u\n", MODBUS_REG_ADDR, modbusValue);
  } else {
    Serial.printf("❌ Modbus Error: 0x%02X\n", event);
  }
  return true;
}

// -------------------- Setup -----------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n🚀 Starting EQSP32 Modbus RTU Master (Non-blocking) ...");

  // Initialize EQSP32
  eqsp32.begin();

  // Retrieve RS485 hardware pin mapping
  mbTx = eqsp32.getPin(EQ_RS485_TX);
  mbRx = eqsp32.getPin(EQ_RS485_RX);
  mbEn = eqsp32.getPin(EQ_RS485_EN);

  pinMode(mbEn, OUTPUT);
  digitalWrite(mbEn, LOW);

  // Setup RS485 communication (Serial2)
  MB_SERIAL.begin(9600, SERIAL_8N1, mbRx, mbTx);
  mb.begin(&MB_SERIAL, mbEn);   // RS485 direction handled automatically
  mb.master();                  // Configure as Modbus RTU master

  Serial.printf("RS485 (Modbus RTU Master) @9600\n");

  tModbusPoll.start();  // Start periodic poll timer
}

// -------------------- Loop ------------------------------
void loop() {
  // === Non-blocking Modbus Polling ===
  if (tModbusPoll.isExpired()) {
    tModbusPoll.reset();
    if (!mb.slave()) {  // Only send new request if no transaction in progress
      mb.readHreg(MODBUS_SLAVE_ID, MODBUS_REG_ADDR, &modbusValue, MODBUS_REG_COUNT, modbusReadCallback);
    }
  }

  mb.task();  // Keep Modbus state machine running

  delay(10);  // FreeRTOS-friendly loop pacing
}
