/**
 * @file EQSP32_ModbusRTU_Slave_Demo.ino
 * @brief Modbus RTU slave example using the EQSP32 onboard RS485 interface (Serial2).
 *
 * This example demonstrates how to configure the EQSP32 as a Modbus RTU slave device
 * using its onboard RS485 transceiver and the `ModbusRTU` library by Alexander Emelianov.
 *
 * Features:
 * - Uses `getPin()` to automatically retrieve and prepare RS485 TX, RX, and EN pin mappings.
 * - Initializes Serial2 for Modbus RTU communication at 9600 bps, 8N1.
 * - Creates one holding register (HR[10]) that can be read or written by a Modbus master.
 * - Fully non-blocking implementation using `mb.task()` for continuous background processing.
 * - Compatible with the `EQSP32_ModbusRTU_Master_Demo` and the `EQSP32R_Dual_RS485_ModbusRTU_Master_DMX` example.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS485 interface (no external transceiver required).
 * - RS485 A and B lines connected to another EQSP32 running as a Modbus master
 *   or to a USB-RS485 adapter running Modbus client software.
 *
 * Wiring:
 *   EQSP32 RS485 A/TX  →  Master RS485 A
 *   EQSP32 RS485 B/RX  →  Master RS485 B
 *   GND                →  GND
 *
 * Notes:
 * - RS485 direction control is handled automatically via the enable pin.
 * - Non-blocking logic ensures smooth FreeRTOS operation with minimal CPU usage.
 * - Default settings: Slave ID = 1, Baud rate = 9600, Parity = None, Stop bits = 1.
 *
 * PC-Side Testing:
 * - Configure as: RTU mode, Slave ID 1, Baud 9600, Data 8, Parity None, Stop 1.
 * - Read Holding Register 10 to view or modify the slave’s stored value.
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

#define SLAVE_ID        1
#define MODBUS_BAUD     9600
#define MODBUS_REG_ADDR 10

EQSP32 eqsp32;
ModbusRTU mb;
HardwareSerial& MB_SERIAL = Serial2;

int txPin, rxPin, enPin;
uint16_t holdingReg10 = 1234;   // initial value

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n🚀 Starting EQSP32 Modbus RTU Slave ...");

  // Initialize EQSP32
  eqsp32.begin();

  // Get RS485 pin mapping
  txPin = eqsp32.getPin(EQ_RS485_TX);
  rxPin = eqsp32.getPin(EQ_RS485_RX);
  enPin = eqsp32.getPin(EQ_RS485_EN);

  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);

  // Initialize Modbus over Serial2
  MB_SERIAL.begin(MODBUS_BAUD, SERIAL_8N1, rxPin, txPin);
  mb.begin(&MB_SERIAL, enPin);
  mb.slave(SLAVE_ID);

  Serial.printf("RS485 (Modbus RTU Slave) @9600\n");

  // Create one holding register (address 10)
  mb.addHreg(MODBUS_REG_ADDR, holdingReg10);

  Serial.printf("✅ Modbus Slave ready (ID=%d, HR[%d]=%d)\n", SLAVE_ID, MODBUS_REG_ADDR, holdingReg10);
}

void loop() {
  mb.task();  // non-blocking Modbus engine

  // keep HR10 updated in memory
  holdingReg10 = mb.Hreg(MODBUS_REG_ADDR);

  delay(10);  // FreeRTOS-friendly loop pacing
}
