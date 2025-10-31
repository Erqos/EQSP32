/**
 * @file EQSP32_DMX512_Demo.ino
 * @brief DMX512 transmitter example using EQSP32 onboard RS485 interface.
 *
 * This example demonstrates how to transmit DMX512 frames using the EQSP32’s
 * built-in RS485 interface (UART2) without requiring any external DMX libraries.
 *
 * Features:
 * - Uses `getPin()` to automatically retrieve RS485 TX, RX, and EN pin mappings.
 * - Generates proper DMX512 timing: BREAK (~120 µs) + MAB (~12 µs) at 250 kbps (8N2).
 * - Continuously transmits 512-channel DMX frames every 50 ms (~20 Hz refresh rate).
 * - Demonstrates how to control typical DMX RGBW fixtures with sequential color fades.
 *
 * Fixture Reference:
 * - Many DMX lights (such as WKL-BEAM02 / Monoprice Pin Spot) use a similar channel layout:
 *      CH1 → Mode / Dimmer / Strobe control
 *      CH2 → Red intensity
 *      CH3 → Green intensity
 *      CH4 → Blue intensity
 *      CH5 → White intensity
 *      CH6 → Auto / Sound control
 * - In this demo, CH1 is set to 255 (Switch mode / always ON) and CH6 is set to 0 (manual RGBW mode).
 * - Channels 2–5 perform a smooth sequential RGBW fade cycle for visual confirmation.
 *
 * Important Timing Note:
 * - DMX devices and fixtures may have an internal timeout and will turn off
 *   automatically if they do not receive valid DMX data frequently enough.
 * - Therefore, DMX frames should be sent at a more frequent rate than the DMX devices' timeout; this example uses 50 ms
 *   (20 Hz) to ensure stable and continuous light output.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS485 interface (no external transceiver required).
 * - Connect RS485 A/B directly to the DMX fixture’s DMX IN terminals.
 *
 * Wiring:
 *   EQSP32 RS485 A/TX  →  DMX IN +
 *   EQSP32 RS485 B/RX  →  DMX IN –
 *   GND                →  Fixture GND
 *
 * @dependencies
 * - EQSP32 library (v1.0.4 or later)
 *
 * @date 2025-10-30
 * @author Erqos Technologies
 */


#include <Arduino.h>
#include <EQSP32.h>

// -------------------- Configuration --------------------
#define DMX_BAUD        250000
#define DMX_BREAK_US    120
#define DMX_MAB_US      12
#define DMX_FRAME_MS    50
#define DMX_CHANNELS    512
#define DMX_STEP        5

// -------------------- Globals --------------------------
EQSP32 eqsp32;

HardwareSerial& DMX_SERIAL = Serial2;
int dmxTx, dmxRx, dmxEn;

uint8_t dmxFrame[DMX_CHANNELS + 1];
EQTimer tDMXFrame(DMX_FRAME_MS);

static uint8_t seqIndex = 0;  // 0=R, 1=G, 2=B, 3=W
static uint8_t level = 0;     // fade level 0–255

// -------------------- DMX Functions ---------------------
void dmxBeginUart() {
  DMX_SERIAL.begin(DMX_BAUD, SERIAL_8N2, dmxRx, dmxTx, false, 1);
}

void dmxSendFrame(uint8_t* packet, int size) {
  digitalWrite(dmxEn, HIGH);
  DMX_SERIAL.flush();
  DMX_SERIAL.end();
  pinMode(dmxTx, OUTPUT);

  // BREAK + MAB
  digitalWrite(dmxTx, LOW);
  delayMicroseconds(DMX_BREAK_US);
  digitalWrite(dmxTx, HIGH);
  delayMicroseconds(DMX_MAB_US);

  // UART back on for data
  dmxBeginUart();
  DMX_SERIAL.write(packet, size);
  DMX_SERIAL.flush();
  digitalWrite(dmxEn, LOW);
}

// -------------------- Setup -----------------------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n🚀 Starting EQSP32 DMX512 Demo ...");

  // Initialize EQSP32
  eqsp32.begin();

  dmxTx = eqsp32.getPin(EQ_RS485_TX);
  dmxRx = eqsp32.getPin(EQ_RS485_RX);
  dmxEn = eqsp32.getPin(EQ_RS485_EN);

  pinMode(dmxEn, OUTPUT);
  digitalWrite(dmxEn, LOW);

  dmxBeginUart();
  memset(dmxFrame, 0, sizeof(dmxFrame));
  dmxFrame[0] = 0x00;  // Start Code

  Serial.printf("RS485 (DMX) @250k\n");

  tDMXFrame.start();
}

// -------------------- Loop ------------------------------
void loop() {
  if (tDMXFrame.isExpired()) {
    tDMXFrame.reset();

    // Sequential fade R → G → B → W
    uint8_t r = 0, g = 0, b = 0, w = 0;
    switch (seqIndex) {
      case 0: r = level; break;
      case 1: g = level; break;
      case 2: b = level; break;
      case 3: w = level; break;
    }

    // --- Build WKL-BEAM02 DMX frame ---
    dmxFrame[1] = 255;  // CH1: Head function (Switch mode - always ON)
    dmxFrame[2] = r;    // CH2: Red dimmer
    dmxFrame[3] = g;    // CH3: Green dimmer
    dmxFrame[4] = b;    // CH4: Blue dimmer
    dmxFrame[5] = w;    // CH5: White dimmer
    dmxFrame[6] = 0;    // CH6: Manual mode (<8)

    dmxSendFrame(dmxFrame, DMX_CHANNELS + 1);

    // Fade progression
    if (level + DMX_STEP >= 255) {
      level = 0;
      seqIndex = (seqIndex + 1) & 0x03;
    } else {
      level += DMX_STEP;
    }
  }

  delay(10);  // FreeRTOS-friendly
}
