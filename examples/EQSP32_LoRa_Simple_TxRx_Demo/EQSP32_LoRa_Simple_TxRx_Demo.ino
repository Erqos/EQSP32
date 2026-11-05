/**
 * @file EQSP32_LoRa_Simple_TxRx_Demo.ino
 * @brief Simple LoRa point-to-point transmit/receive demo using the EQSP32 onboard SX1262 LoRa interface.
 *
 * This example demonstrates how to:
 * - Initialize the EQSP32 system using the standard `eqsp32.begin()` startup flow.
 * - Check whether the onboard LoRa module is available before starting LoRa communication.
 * - Configure the LoRa radio parameters such as frequency, bandwidth, spreading factor,
 *   coding rate, sync word, output power, and preamble length.
 * - Flash one EQSP32 as a transmitter and another EQSP32 as a receiver.
 * - Send periodic text packets from the transmitter.
 * - Receive LoRa packets on the receiver and print the message, RSSI, and SNR.
 *
 * Operation:
 * - Select the firmware mode before flashing:
 *   - Enable `DEMO_MODE_TX` for the transmitting EQSP32.
 *   - Enable `DEMO_MODE_RX` for the receiving EQSP32.
 * - The transmitter sends a message every `TX_INTERVAL_MS`.
 * - The receiver continuously listens for incoming packets.
 * - Received packets are printed to the Serial Monitor together with signal quality data.
 *
 * Hardware Setup:
 * - Two EQSP32 controllers with supported onboard SX1262 LoRa hardware.
 * - Suitable LoRa antennas connected to both units.
 * - Both units must use the same LoRa settings:
 *   frequency, bandwidth, spreading factor, coding rate, sync word, and preamble length.
 *
 * Features:
 * - Uses the EQSP32 library-managed LoRa object `eqsp32LoRa`.
 * - Validates LoRa availability using `eqsp32.isLoRaAvailable()`.
 * - Uses RadioLib-style return codes for initialization, transmit, and receive status.
 * - Enables LoRa CRC for basic packet integrity checking.
 * - Non-blocking TX timing using the library's `EQTimer`.
 * - FreeRTOS-friendly loop pacing using `delay(10)`.
 *
 * Notes:
 * - Radio rules: this example uses 868.1 MHz and sends one message every 10 s. In the EU,
 *   the 868.0-868.6 MHz band allows a device to transmit at most 1 % of the time (36 s per hour).
 *   One message of this example takes about 60 ms on air, so one every 10 s uses about 0.6 %.
 *   Sending more often, longer messages, or a higher spreading factor (longer airtime) can
 *   exceed the limit. Outside the EU, use the band and rules of your region (for example
 *   915 MHz in the US). Verify that the selected frequency, transmit power, duty cycle, and
 *   antenna setup comply with local radio regulations.
 * - Both TX and RX devices must be configured with matching LoRa parameters.
 * - This example is intended for basic point-to-point LoRa testing only; it does not implement
 *   addressing, acknowledgements, retries, encryption, or packet framing.
 * - If `eqsp32.isLoRaAvailable()` returns false, the LoRa module is not detected or not ready,
 *   and LoRa operations should not be attempted.
 *
 * @author Erqos Technologies
 * @date 2026-06-15
 */

#include <EQSP32.h>

EQSP32 eqsp32;

// Select firmware mode
#define DEMO_MODE_TX
// #define DEMO_MODE_RX


#if defined(DEMO_MODE_TX) && defined(DEMO_MODE_RX)
  #error "Select only one mode: DEMO_MODE_TX or DEMO_MODE_RX"
#elif !defined(DEMO_MODE_TX) && !defined(DEMO_MODE_RX)
  #error "Select DEMO_MODE_TX or DEMO_MODE_RX"
#endif


#define LORA_FREQ       868.1   // MHz, inside the EU 868.0-868.6 MHz band (see Notes)
#define LORA_BW         125.0
#define LORA_SF         7
#define LORA_CR         5
#define LORA_SYNC       0x12
#define LORA_POWER      2 	// Ra-01SH-P LoRa module is used internally with max recommended power 3
#define LORA_PREAMBLE   8

#define TX_INTERVAL_MS  10000   // One message every 10 s: about 0.6 % airtime, under the EU 1 % limit

uint32_t txCounter = 0;
EQTimer txTimer(TX_INTERVAL_MS);    // Paces the transmissions

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("\nEQSP32 LoRa Simple TX/RX Demo");

  eqsp32.begin();  // Initializes the EQSP32 core and module handling

  if (!eqsp32.isLoRaAvailable()) {
    Serial.println("LoRa module not available.");
    while (true) {
      delay(1000);
    }
  }

  int16_t state = eqsp32LoRa.begin(
    LORA_FREQ,
    LORA_BW,
    LORA_SF,
    LORA_CR,
    LORA_SYNC,
    LORA_POWER,
    LORA_PREAMBLE
  );

  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("LoRa begin failed, code=%d\n", state);
    while (true) {
      delay(1000);
    }
  }

  eqsp32LoRa.setCRC(true);

#if defined(DEMO_MODE_TX)
  Serial.println("Mode: TX");
  Serial.printf("Sending one message every %d s.\n", TX_INTERVAL_MS / 1000);
  txTimer.start();
#elif defined(DEMO_MODE_RX)
  Serial.println("Mode: RX");
#else
  #error "Select DEMO_MODE_TX or DEMO_MODE_RX"
#endif

  Serial.println("LoRa ready.");
}

void loop() {
#if defined(DEMO_MODE_TX)

  if (txTimer.isElapsed(true)) {    // true: the timer starts the next period at once

    char message[48];
    snprintf(message, sizeof(message), "Hello from EQSP32 #%lu", (unsigned long)txCounter++);

    int16_t state = eqsp32LoRa.transmit(message);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.printf("[TX] Sent: %s\n", message);
    } else {
      Serial.printf("[TX] Failed, code=%d\n", state);
    }
  }

#elif defined(DEMO_MODE_RX)

  String message;
  int16_t state = eqsp32LoRa.receive(message);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.printf("[RX] %s | RSSI=%.1f dBm | SNR=%.1f dB\n",
                  message.c_str(),
                  eqsp32LoRa.getRSSI(),
                  eqsp32LoRa.getSNR());
  } else if (state != RADIOLIB_ERR_RX_TIMEOUT) {
    Serial.printf("[RX] Error, code=%d\n", state);
  }

#endif

  delay(10);
}