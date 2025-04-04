/**
 * @file EQSP32_CAN-Bus_Demo.ino
 * @brief Demonstrates CAN Bus communication using EQSP32 library functions.
 *
 * This example:
 * - Initializes CAN interface at 500 kbps.
 * - Supports three configuration modes:
 *   1. Accept all messages
 *   2. Accept all + loopback (self-testing)
 *   3. Filter exact CAN ID
 *
 * - Sends a CAN message every 2 seconds.
 * - Receives and prints CAN messages if available.
 *
 * Hardware:
 * - EQSP32 connected to a CAN bus network via CAN_TX/CAN_RX pins.
 * - Ensure 120-ohm termination at both ends of the CAN bus.
 *
 * @date 2025-04-04
 * @author Erqos Technologies
 */

#include <EQSP32.h>

EQSP32 eqsp32;

// === Configuration Constants ===
#define CAN_CONFIG      1       // Set to 1, 2, or 3 to select mode
#define SEND_INTERVAL   2000    // Transmit a frame every 2 seconds

uint32_t exactCanId = 0x123;    // Used in CAN_CONFIG 3 of this example
uint8_t messageCounter = 0;

EQTimer txFrameTimer(SEND_INTERVAL);

/**
 * @brief Configure the CAN interface with the selected mode.
 *
 * Modes:
 * 1. Accept All IDs: receives all CAN messages from other nodes.
 *    eqsp32.configCAN(CAN_500K);
 *
 * 2. Loopback + Accept All: used for self-testing — receive even your own transmitted frames.
 *    eqsp32.configCAN(CAN_500K, 0, true);
 *
 * 3. Filter by exact Standard CAN ID:
 *    eqsp32.configCAN(CAN_500K, 0x123);
 *    // Only receives messages with identifier == 0x123
 */
bool initCANBus() {
    switch (CAN_CONFIG) {
        case 1:
            Serial.println("🔧 Mode 1: Accept All Messages");
            return eqsp32.configCAN(CAN_500K);  // Accept all incoming messages

        case 2:
            Serial.println("🔧 Mode 2: Accept All + Loopback");
            return eqsp32.configCAN(CAN_500K, 0, true);   // Accept all + loopback (also accepts own messages)

        case 3:
            Serial.println("🔧 Mode 3: Filter by Exact ID (0x123)");
            return eqsp32.configCAN(CAN_500K, exactCanId);  // Exact ID (only receives ID 0x123), not own messages

        default:
            Serial.println("❌ Invalid CAN_CONFIG. Set to 1, 2, or 3.");
            return false;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚗 Starting EQSP32 CAN Bus Demo...");

    eqsp32.begin();  // EQSP32 setup (non-verbose)

    if (initCANBus()) {
        Serial.println("✅ CAN interface initialized successfully.");
    } else {
        Serial.println("❌ Failed to initialize CAN interface.");
        while (true);  // Halt
    }

    txFrameTimer.start();
}

void loop() {
    // Transmit every 2 seconds
    if (txFrameTimer.isExpired()) {
        txFrameTimer.reset();
        transmitForCANMessage();
    }

    checkForCANMessage();

    delay(10);  // FreeRTOS-friendly loop delay
}

void transmitForCANMessage() {
    CanMessage txMsg = {};
    txMsg.identifier = 0x120;  // Default outgoing ID; may not be received if filtered
    txMsg.data_length_code = 4;
    txMsg.data[0] = messageCounter++;
    txMsg.data[1] = 0x11;
    txMsg.data[2] = 0x22;
    txMsg.data[3] = 0x33;

    if (eqsp32.transmitCANFrame(txMsg)) {
        Serial.printf("✉️🡆\tTX: ID=0x%03X\tData=[ %02X %02X %02X %02X ]\n",
                      txMsg.identifier, txMsg.data[0], txMsg.data[1],
                      txMsg.data[2], txMsg.data[3]);
    } else {
        Serial.println("⚠️ Failed to send CAN message.");
    }
}

void checkForCANMessage() {
    CanMessage rxMsg;
    if (eqsp32.receiveCANFrame(rxMsg)) {
        Serial.printf("🡄✉️\tRX: ID=0x%03X\tData=[ ", rxMsg.identifier);
        for (int i = 0; i < rxMsg.data_length_code; ++i) {
            Serial.printf("%02X ", rxMsg.data[i]);
        }
        Serial.printf("]\tDLC=%d\n", rxMsg.data_length_code);
    }
}
