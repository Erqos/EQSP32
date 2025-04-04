/**
 * @file EQSP32_CAN-Bus_getPin_Demo.ino
 * @brief CAN communication using TWAI library with EQSP32 pin mapping.
 *
 * This example:
 * - Initializes CAN interface at 500 kbps.
 * - Accept all messages (not own messages).
 *
 * Hardware:
 * - EQSP32 connected to a CAN bus network via CAN_TX/CAN_RX pins.
 * - Ensure 120-ohm termination at both ends of the CAN bus.
 *
 * @date 2025-04-04
 * @author Erqos Technologies
 */

#include <EQSP32.h>
#include "driver/twai.h"  // ESP32-S3 TWAI driver

EQSP32 eqsp32;

#define SEND_INTERVAL   2000    // Transmit a frame every 2 seconds

int canTxPin, canRxPin;
uint8_t messageCounter = 0;

EQTimer txFrameTimer(SEND_INTERVAL);

void setup() {
    Serial.begin(115200);
    Serial.println("\n🚗 Starting EQSP32 CAN Bus TWAI Demo ...");

    // Initialize EQSP32
    eqsp32.begin();

    // Get the CAN TX/RX pin numbers from the EQSP32 object
    canTxPin = eqsp32.getPin(EQ_CAN_TX);
    canRxPin = eqsp32.getPin(EQ_CAN_RX);

    // Setup TWAI configs
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)canTxPin, (gpio_num_t)canRxPin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();  // Accept all messages

    // Install and start TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK && twai_start() == ESP_OK) {
        Serial.println("✅ CAN (TWAI) interface started at 500 kbps.");
    } else {
        Serial.println("❌ Failed to initialize CAN interface.");
        while (true);  // Stop here
    }

    txFrameTimer.start();
}

void loop() {
    // Transmit every 2 seconds
    if (txFrameTimer.isExpired()) {
        txFrameTimer.reset();
        transmitCANMessage();
    }

    receiveCANMessage();
    delay(10);  // FreeRTOS-safe delay
}

void transmitCANMessage() {
    twai_message_t tx_msg = {};
    tx_msg.identifier = 0x120;
    tx_msg.data_length_code = 4;
    tx_msg.data[0] = messageCounter++;
    tx_msg.data[1] = 0x11;
    tx_msg.data[2] = 0x22;
    tx_msg.data[3] = 0x33;

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.printf("✉️🡆\tTX: ID=0x%03X\tData=[ %02X %02X %02X %02X ]\n",
                      tx_msg.identifier, tx_msg.data[0], tx_msg.data[1],
                      tx_msg.data[2], tx_msg.data[3]);
    } else {
        Serial.println("⚠️ Failed to transmit CAN frame.");
    }
}

void receiveCANMessage() {
    twai_message_t rx_msg;
    if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.printf("🡄✉️\tRX: ID=0x%03X\tData=[ ", rx_msg.identifier);
        for (int i = 0; i < rx_msg.data_length_code; i++) {
            Serial.printf("%02X ", rx_msg.data[i]);
        }
        Serial.printf("]\tDLC=%d\n", rx_msg.data_length_code);
    }
}
