/**
 * @file EQSP32_RS485_getPin_Demo.ino
 * @brief RS485 communication using EQSP32 internal UART2 and onboard RS485 transceiver.
 *
 * This example:
 * - Uses `getPin()` to retrieve RS485_TX, RS485_RX, and RS485_EN pin mappings.
 * - `getPin()` automatically prepares EQSP32 serial for RS485 communication.
 * - Initializes Serial2 at 9600 bps and maps it to the RS485 peripheral pins.
 * - Demonstrates how to send and receive messages over RS485 using built-in hardware.
 *
 * Hardware Setup:
 * - EQSP32 with onboard RS485 interface (no external transceiver needed).
 * - USB-RS485 adapter connected to a PC for testing.
 *
 * Wiring:
 * - EQSP32 RS485 A/TX  -> Adapter RS485 A
 * - EQSP32 RS485 B/RX  -> Adapter RS485 B
 *
 * PC side: Open a serial terminal (e.g., PuTTY, RealTerm) to view or send RS485 messages.
 *
 * @date 2025-05-14
 * @author Erqos Technologies
 */


#include <EQSP32.h>

EQSP32 eqsp32;

int txPin, rxPin, enPin;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n🔁 Starting EQSP32 RS485 Demo...");

    // Initialize EQSP32 system
    eqsp32.begin();

    // Retrieve native GPIO numbers for RS485 UART (also prepares EQSP32 hardware for RS485 communication)
    txPin = eqsp32.getPin(EQ_RS485_TX);
    rxPin = eqsp32.getPin(EQ_RS485_RX);
    enPin = eqsp32.getPin(EQ_RS485_EN);

    // Setup RS485 transceiver direction control
    pinMode(enPin, OUTPUT);
    digitalWrite(enPin, LOW);  // Default to receive mode

    // Initialize UART2 for RS485 communication
    Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);

    // Send a startup message to RS485 line
    sendRS485Message("Hello RS485 device!");
}

void loop() {
    // Monitor incoming RS485 messages
    if (Serial2.available()) {
        String msg = Serial2.readStringUntil('\n');
        msg.trim();
        if (msg.length()) {
            Serial.print("✅ RS485 received: ");
            Serial.println(msg);
        }
    }

    delay(10);  // FreeRTOS-compatible loop pacing
}

// Send string over RS485 (half-duplex)
void sendRS485Message(const String& message) {
    digitalWrite(enPin, HIGH);  // Enable transmitter
    delayMicroseconds(100);     // Settle time for transceiver

    Serial2.println(message);   // Transmit message
    Serial.print("✉️🡆\tRS485 sent: ");
    Serial.println(message);

    Serial2.flush();            // Wait for transmission to complete
    delay(2);                   // Ensure trailing data clears

    digitalWrite(enPin, LOW);   // Return to receive mode
}
