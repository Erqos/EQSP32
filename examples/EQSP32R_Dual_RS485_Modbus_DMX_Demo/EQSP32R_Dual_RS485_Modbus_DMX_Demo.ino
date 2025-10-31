/**
 * @file EQSP32R_Dual_RS485_ModbusRTU_Master_DMX_Demo.ino
 * @brief Dual RS485 asynchronous communication example for EQSP32R (Modbus RTU Master + DMX).
 *
 * This example demonstrates simultaneous operation of the two independent RS485
 * peripherals available on the EQSP32R (Dual RS485 version, instead of CAN).
 *
 * Features:
 * - Uses `getPin()` to automatically retrieve and configure pin mappings for both RS485 peripherals:
 *      - RS485 #1 (EQ_RS485_TX / RX / EN) for Modbus RTU communication.
 *      - RS485 #2 (EQ_RS485_2_TX / RX / EN) for DMX data transmission.
 * - Implements a **non-blocking Modbus RTU master** using the `ModbusRTU` library by Alexander Emelianov.
 * - Continuously calls `mb.task()` in the main loop to process Modbus communication asynchronously.
 * - Periodically requests Holding Register 10 from a Modbus slave every second, using callback-based response handling.
 * - Implements a lightweight DMX transmitter at 250000 bps (8N2), continuously sending 512-channel frames every 50 ms.
 * - Demonstrates concurrent operation of both RS485 interfaces under FreeRTOS without blocking or timing jitter.
 * - Maintains continuous DMX output even when the Modbus slave is disconnected or unresponsive.
 *
 * Hardware Setup:
 * - EQSP32R (Dual RS485 version; no external RS485 transceiver required).
 * - RS485 #1 connected to a Modbus RTU slave device or USB-RS485 adapter running Modbus software.
 * - RS485 #2 connected to a DMX fixture or DMX USB interface for monitoring output.
 *
 * Wiring:
 *   RS485 #1 (Modbus):
 *     - EQSP32R RS485 A/TX  -> Slave or Adapter RS485 A
 *     - EQSP32R RS485 B/RX  -> Slave or Adapter RS485 B
 *
 *   RS485 #2 (DMX):
 *     - EQSP32R RS485 2 A/TX  -> DMX IN +
 *     - EQSP32R RS485 2 B/RX  -> DMX IN –
 *
 * Notes:
 * - `getPin()` automatically initializes and assigns the correct RS485 pin mappings for both ports.
 * - RS485 #1 (Serial1) and RS485 #2 (Serial2) operate on independent UART peripherals with separate enable controls.
 * - Modbus RTU master runs asynchronously at 9600 bps (8N1), polling slave ID 1 for Holding Register 10 once per second.
 * - DMX transmitter runs at 250 kbps (8N2) and sends full 512-channel frames every 50 ms (≈20 Hz).
 * - DMX output cycles color channels sequentially (Red → Green → Blue → White) with smooth intensity transitions.
 * - Both Modbus and DMX loops are scheduled via `EQTimer` for simple, non-blocking timing.
 * - Example remains stable even if the Modbus slave is disconnected, ensuring uninterrupted DMX output.
 * - Fully FreeRTOS-safe with minimal CPU load (loop delay ≈ 10 ms).
 *
 * PC-Side Testing:
 * - For Modbus: connect either an EQSP32 running the Modbus RTU slave example or a USB-RS485 adapter
 *   with Modbus simulation software (e.g., Simply Modbus Slave).
 *   Configure as: Slave ID = 1, Baud = 9600, Data = 8, Parity = None, Stop Bits = 1, and monitor Holding Register 10.
 * - For DMX: connect a DMX-compatible fixture set to Start Address 1, or use a DMX monitoring tool
 *   to visualize the sequential color fading on channels 2–5.
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
#include <ModbusRTU.h>        // by Alexander Emelianov

// -------------------- Configuration --------------------
#define MODBUS_SLAVE_ID   1
#define MODBUS_REG_ADDR   10
#define MODBUS_REG_COUNT  1
#define MODBUS_POLL_MS    1000

#define DMX_BAUD          250000
#define DMX_BREAK_US      120
#define DMX_MAB_US        12
#define DMX_FRAME_MS      50
#define DMX_CHANNELS      512

// -------------------- Globals --------------------------
EQSP32 eqsp32;

// RS485 #1  (Modbus)
ModbusRTU mb;
HardwareSerial& MB_SERIAL = Serial1;
int mbTx, mbRx, mbEn;

// RS485 #2  (DMX)
HardwareSerial& DMX_SERIAL = Serial2;
int dmxTx, dmxRx, dmxEn;
uint8_t dmxFrame[DMX_CHANNELS + 1];

// Timers
EQTimer tModbusPoll(MODBUS_POLL_MS);   // Modbus poll every 1s (or whatever you set)
EQTimer tDMXPoll(DMX_FRAME_MS);      // DMX frame rate (~30 ms for ~33 Hz)

// Brightness sequencer
static uint8_t seqIndex = 0;     // 0=R, 1=G, 2=B, 3=W
static uint8_t level    = 0;     // 0..255
static const uint8_t STEP = 5;   // fade step per DMX frame (adjust to taste)

// -------------------- DMX helpers -----------------------
void dmxBeginUart() {
  DMX_SERIAL.begin(DMX_BAUD, SERIAL_8N2, dmxRx, dmxTx, false, 1);
}

void dmxSendFrame(uint8_t* packet, int size) {
  // Bus preparation
  digitalWrite(dmxEn, HIGH);
  DMX_SERIAL.flush();
  DMX_SERIAL.end();
  pinMode(dmxTx, OUTPUT);

  digitalWrite(dmxTx, LOW);       // BREAK
  delayMicroseconds(DMX_BREAK_US);
  digitalWrite(dmxTx, HIGH);      // MAB
  delayMicroseconds(DMX_MAB_US);

  // RS485 serial preparation
  dmxBeginUart();

  // Send DMX 512 frame
  DMX_SERIAL.write(packet, size);
  DMX_SERIAL.flush();
  digitalWrite(dmxEn, LOW);
}

// -------------------- Modbus callback -------------------
uint16_t modbusValue = 0;

bool modbusReadCallback(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.printf("✅ Modbus HR[%u] = %u\n", MODBUS_REG_ADDR, modbusValue);
  } else {
    Serial.printf("❌ Modbus error: 0x%02X\n", event);
  }
  return true;  // continue processing
}

// -------------------- Setup -----------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n🚀 Starting EQSP32R Dual RS485 Demo (Modbus RTU Master and DMX) ...");

  // Initialize EQSP32
  eqsp32.begin();

  // --- RS485 #1 (Modbus RTU) ---
  mbTx = eqsp32.getPin(EQ_RS485_TX);    // Get main RS485 pin control
  mbRx = eqsp32.getPin(EQ_RS485_RX);
  mbEn = eqsp32.getPin(EQ_RS485_EN);

  pinMode(mbEn, OUTPUT);
  digitalWrite(mbEn, LOW);

  MB_SERIAL.begin(9600, SERIAL_8N1, mbRx, mbTx);
  mb.begin(&MB_SERIAL, mbEn);   // enable pin handles direction automatically
  mb.master();                  // configure as master

  // --- RS485 #2 (DMX) ---
  dmxTx = eqsp32.getPin(EQ_RS485_2_TX);   // Get second RS485 pin control
  dmxRx = eqsp32.getPin(EQ_RS485_2_RX);
  dmxEn = eqsp32.getPin(EQ_RS485_2_EN);

  pinMode(dmxEn, OUTPUT);
  digitalWrite(dmxEn, LOW);
  dmxBeginUart();

  memset(dmxFrame, 0, sizeof(dmxFrame));  // Reserve memory for DMX frame buffer
  dmxFrame[0] = 0x00;  // Start code

  Serial.printf("RS485 #1 : Modbus RTU Master @9600\t (Mapped to Serial1)\n");
  Serial.printf("Rs485 #2 : DMX               @250k\t (Mapped to Serial2)\n");

  tModbusPoll.start();
  tDMXPoll.start();
}

// -------------------- Loop ------------------------------
void loop() {
  // === Non-blocking Modbus via EQTimer ===
  if (tModbusPoll.isExpired()) {
    tModbusPoll.reset();
    if (!mb.slave()) {
      // asynchronous request; completion checked by callback OR mb.lastResult() if you went callback-less
      mb.readHreg(MODBUS_SLAVE_ID, MODBUS_REG_ADDR, &modbusValue, MODBUS_REG_COUNT, modbusReadCallback);
    }
  }
  mb.task();   // keep Modbus state machine running (non-blocking)

  // === DMX output via EQTimer (continuous frames) ===
  if (tDMXPoll.isExpired()) {
    tDMXPoll.reset();

    // Build one-at-a-time color fade: R -> G -> B -> W
    uint8_t r = 0, g = 0, b = 0, w = 0;
    switch (seqIndex) {
      case 0: r = level; break;
      case 1: g = level; break;
      case 2: b = level; break;
      case 3: w = level; break;
    }

    dmxFrame[1] = 255; // CH1 (Function/Dimmer band). If you insist on 255, set to 255.
    dmxFrame[2] = r;                    // CH2 Red
    dmxFrame[3] = g;                    // CH3 Green
    dmxFrame[4] = b;                    // CH4 Blue
    dmxFrame[5] = w;                    // CH5 White
    dmxFrame[6] = 0;                    // CH6 Auto/Sound off (<8 keeps manual)

    dmxSendFrame(dmxFrame, DMX_CHANNELS + 1);

    // Advance fade
    if (level + STEP >= 255) {
      level = 0;
      seqIndex = (seqIndex + 1) & 0x03; // 0..3
    } else {
      level += STEP;
    }
  }

  // FreeRTOS-friendly pacing
  delay(10);
}


