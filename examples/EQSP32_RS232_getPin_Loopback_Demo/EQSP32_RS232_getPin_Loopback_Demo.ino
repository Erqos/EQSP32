/**
 * @file EQSP32_RS232_getPin_Loopback_Demo.ino
 * @brief RS232 loopback test using EQSP32 internal UART2 and onboard RS232 mapping.
 *
 * This example:
 * - Uses `getPin()` to retrieve RS232_TX and RS232_RX pin mappings.
 * - Initializes Serial2 at 9600 bps and maps it to the RS232 hardware pins.
 * - Periodically transmits a test string and listens for it on the receive line.
 * - Verifies that the sent and received messages match.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS232 interface.
 * - Loopback test: connect RS232 Serial A/TX to Serial B/RX externally.
 *
 * @date 2025-05-14
 * @author Erqos Technologies
 */

#include <EQSP32.h>

EQSP32 eqsp32;

int txPin, rxPin;
int msgCounter = 0;
String lastSent = "";
String lastReceived = "";

EQTimer loopbackTimer(2000);  // Every 2 seconds

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n🔁 Starting EQSP32 RS232 Loopback Test...");

  eqsp32.begin();

  txPin = eqsp32.getPin(EQ_RS232_TX);
  rxPin = eqsp32.getPin(EQ_RS232_RX);

  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);

  loopbackTimer.start();
}

void loop() {
  if (loopbackTimer.isExpired()) {
    loopbackTimer.reset();

    // Create and send new test message
    lastSent = "RS232 Test " + String(msgCounter++);
    Serial2.println(lastSent);
    Serial.print("✉️🡆\tSent: ");
    Serial.println(lastSent);
  }

  if (Serial2.available()) {
    lastReceived = Serial2.readStringUntil('\n');
    lastReceived.trim();
    if (lastReceived.length()) {
      Serial.print("🡄✉️\tReceived: ");
      Serial.println(lastReceived);

      if (lastReceived == lastSent) {
        Serial.println("✅ Match confirmed.");
      } else {
        Serial.println("❌ Mismatch!");
      }
    }
  }

  delay(10);  // FreeRTOS-friendly loop timing
}
