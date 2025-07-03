/**
 * @file EQSP32_CAN-Bus_Demo.ino
 * @brief Demonstrates CAN Bus communication using EQSP32 library functions.
 *
 * This example shows how to:
 * - Initialize the CAN interface at 500 kbps.
 * - Configure different filter modes:
 *   1. Accept all messages
 *   2. Accept all messages with loopback (self-test)
 *   3. Filter by exact Standard CAN ID
 *   4. Filter by CANopen-style Node ID (accept all PDOs/SDOs for that node)
 *
 * - Send a CAN message every 2 seconds.
 * - Receive and print any matching CAN messages to the Serial Monitor.
 *
 * Hardware requirements:
 * - EQSP32 connected to a CAN bus network via CAN_TX/CAN_RX pins.
 * - Ensure proper 120-ohm termination at both ends of the CAN bus.
 *
 * Note:
 * - The Node ID filter in Mode 4 matches all Function Codes for the given Node ID,
 *   but does not receive broadcast frames like SYNC or NMT, which use Node ID 0.
 *
 * @date 2025-07-03
 * @author Erqos Technologies
 */


#include <EQSP32.h>

EQSP32 eqsp32;

// === Configuration Constants ===
// CAN_CONFIG: Choose one filter mode:
// 1 = Accept all CAN messages
// 2 = Accept all + loopback (self-receive)
// 3 = Filter by exact CAN ID
// 4 = Filter by Node ID (CANopen-style, accepts all Function Codes for that node)
#define CAN_CONFIG      1       // Set to 1, 2, 3, or 4 to select mode
#define SEND_INTERVAL   2000    // Transmit a frame every 2 seconds

uint32_t txCanId = 0x120;    // Used in transmitForCANMessage() of this example
uint32_t rxCanId = 0x121;    // Used in CAN_CONFIG 3 of this example
uint8_t ownNodeId = 0x11;    // Example Node ID for CAN_CONFIG 4
uint8_t receiverNodeId = 0x21;	// The receiver's Node ID for sending test PDO messages
uint8_t messageCounter = 0;

EQTimer txFrameTimer(SEND_INTERVAL);


void transmitPDOFrame(uint8_t pdoIndex, uint8_t rxNodeID);
void transmitForCANMessage();
void checkForCANMessage();


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
 *
 * 4. Filter by Node ID (CANopen-style):
 *    eqsp32.configCANNode(CAN_500K, 0x11);		// Accepts all PDOs, SDOs, EMCY for Node ID 0x11
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
			Serial.printf("🔧 Mode 3: Filter by Exact ID (%u)\n", rxCanId);
            return eqsp32.configCAN(CAN_500K, rxCanId);  // Exact ID (only receives rxCanId), not own messages
			
        case 4:
            Serial.printf("🔧 Mode 4: Filter by Node ID (%u)\n", ownNodeId);
            return eqsp32.configCANNode(CAN_500K, ownNodeId);

        default:
            Serial.println("❌ Invalid CAN_CONFIG. Set to 1, 2, 3 or 4.");
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
		
        transmitForCANMessage();	// Send CAN message to txCanId
		
		for (int pdo=1; pdo<=4; pdo++)
			transmitPDOFrame(pdo, receiverNodeId);	// Send PDO messages to receiverNodeId
    }

    checkForCANMessage();

    delay(10);  // FreeRTOS-friendly loop delay
}

/**
 * @brief Transmit a CANopen-style PDO frame for the given PDO index and receiver Node ID.
 *
 * This helper shows how COB-IDs are built in CANopen:
 *   COB-ID = Base + Node ID
 *   TPDO1: 0x180 + Node ID
 *   TPDO2: 0x280 + Node ID
 *   TPDO3: 0x380 + Node ID
 *   TPDO4: 0x480 + Node ID
 *
 * @note
 *   The final COB-ID is (base + Node ID).
 *   Example: TPDO1 to Node ID 0x21 → COB-ID 0x1A1.
 *   If using configCANNode(0x11), only PDOs for Node ID 0x11 pass.
 *
 * @param pdoIndex PDO index (1–4)
 * @param rxNodeID CANopen receiver Node ID (1–127)
 */
void transmitPDOFrame(uint8_t pdoIndex, uint8_t rxNodeID) {
    // Validate parameters
    if (pdoIndex < 1 || pdoIndex > 4 || rxNodeID == 0 || rxNodeID > 127) {
        Serial.println("⚠️ Invalid PDO index or Node ID. Must be TPDO 1-4, Node ID 1-127.");
        return;
    }

    // Calculate base COB-ID for the selected PDO index
    // TPDO1 base: 0x180, TPDO2: 0x280, TPDO3: 0x380, TPDO4: 0x480
    uint32_t baseCOBID = 0x180 + ((pdoIndex - 1) * 0x100);

    // Final COB-ID includes receiver Node ID
    uint32_t cobID = baseCOBID + rxNodeID;

    // Create and populate CAN message
    CanMessage txMsg = {};
    txMsg.identifier = cobID;
    txMsg.data_length_code = 4;  // Example: 4 data bytes
    txMsg.data[0] = messageCounter++;  // Incrementing counter for demo
    txMsg.data[1] = 0xAA;              // Example data
    txMsg.data[2] = 0xBB;
    txMsg.data[3] = 0xCC;

    // Transmit frame
    if (eqsp32.transmitCANFrame(txMsg)) {
        Serial.printf("✉️🡆\tTX PDO%u: COB-ID=0x%03X\tData=[ %02X %02X %02X %02X ]\n",
                      pdoIndex, txMsg.identifier,
                      txMsg.data[0], txMsg.data[1],
                      txMsg.data[2], txMsg.data[3]);
    } else {
        Serial.println("⚠️ Failed to send PDO frame.");
    }
}

/**
 * @brief Transmit a basic CAN frame to txCanId.
 * Used to test standard ID filter or accept-all modes.
 */
void transmitForCANMessage() {
    CanMessage txMsg = {};
    txMsg.identifier = txCanId;  // Default outgoing ID; may not be received if filtered
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
