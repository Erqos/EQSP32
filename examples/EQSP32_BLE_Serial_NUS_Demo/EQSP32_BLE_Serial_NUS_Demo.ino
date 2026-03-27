/**
 * @file EQSP32_BLE_Serial_NUS_Demo.ino
 * @brief Demonstrates the EQSP32 BLE Serial feature using a Nordic UART Service (NUS) style interface.
 *
 * This example assumes:
 *   - Erqos IoT functionality is enabled.
 *   - BLE is managed by the EQSP32 library.
 *   - The EQConnect BLE serial terminal will be used only for feature testing and validation.
 *
 * Intended Use:
 *   - This BLE serial interface is intended for custom BLE-based implementations.
 *   - The EQConnect BLE serial terminal is provided as a convenient tool to test and validate
 *     the BLE serial link during development.
 *
 * BLE Status Monitoring:
 *   - Uses `eqsp32.getBleStatus()` to monitor the BLE state.
 *   - Reports when BLE is not initialized, advertising, connected, or subscribed.
 *   - The BLE serial stream is considered ready when status becomes `EQ_BLE_SUBSCRIBED`.
 *
 * BLE Serial Behavior:
 *   - Sends a periodic test message over BLE serial.
 *   - Forwards data from the USB serial port to BLE.
 *   - Reads data from BLE and echoes it back.
 *   - Prints received BLE data to the USB serial monitor.
 *
 * Testing:
 *   - Open the USB serial monitor at 115200 baud.
 *   - Use the EQConnect BLE serial terminal to connect to the EQSP32.
 *   - Once BLE status becomes `EQ_BLE_SUBSCRIBED`, test bidirectional communication
 *     between USB serial and BLE serial.
 *
 * @author Erqos Technologies
 * @date 2026-03-24
 */

#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer bleTxTimer(1000);

static EQ_BleStatus prevBleStatus = EQ_BLE_NO_INIT;
static bool wasStreamReady = false;

void printBleStatus(EQ_BleStatus status)
{
    switch (status)
    {
        case EQ_BLE_DISCONNECTED:
            Serial.println("BLE initialized but not connected and not advertising.");
            break;

        case EQ_BLE_CONNECTED:
            Serial.println("BLE client connected (not subscribed to notifications yet).");
            break;

        case EQ_BLE_ADVERTISING:
            Serial.println("BLE advertising and discoverable.");
            break;

        case EQ_BLE_SUBSCRIBED:
            Serial.println("BLE client subscribed to notifications. BLE serial stream is ready.");
            break;

        case EQ_BLE_NO_INIT:
            Serial.println("BLE not initialized or not managed by the EQSP32 library.");
            break;

        default:
            Serial.println("Unknown BLE status.");
            break;
    }
}

bool isBleStreamReady(EQ_BleStatus status)
{
    return (status == EQ_BLE_SUBSCRIBED);
}

void handleBleSerialBridge(EQ_BleStatus status)
{
    const bool streamReady = isBleStreamReady(status);

    if (streamReady)
    {
        // Send a periodic test message over BLE serial once per second
        if (bleTxTimer.isExpired())
        {
            bleTxTimer.reset();
            bleSerial.print("tick ");
            bleSerial.println((int)millis());
        }

        // Forward USB serial -> BLE serial
        while (Serial.available())
        {
            int b = Serial.read();
            if (b >= 0)
            {
                uint8_t u = (uint8_t)b;
                bleStream.write(&u, 1);
            }
        }

        // Echo BLE serial -> BLE serial and USB serial
        while (bleSerial.available())
        {
            int b = bleSerial.read();
            if (b >= 0)
            {
                bleSerial.write((uint8_t)b);
                Serial.write((uint8_t)b);
            }
        }
    }
    else
    {
        // Optional: discard pending USB serial bytes while stream is not ready
        while (Serial.available())
            Serial.read();
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println("EQSP32 BLE Serial Example");
    Serial.println("-------------------------");
    Serial.println("Use the EQConnect BLE serial terminal to connect and test BLE serial.");
    Serial.println("BLE serial stream is considered ready when status becomes EQ_BLE_SUBSCRIBED.");
    Serial.println();

    // Default startup with Erqos IoT enabled
    eqsp32.begin();

    bleTxTimer.start();

    Serial.println("Waiting for BLE status changes...");
}

void loop()
{
    // Read current BLE status
    EQ_BleStatus currentStatus = eqsp32.getBleStatus();

    // Print status changes
    if (currentStatus != prevBleStatus)
    {
        Serial.print("BLE Status Changed: ");
        printBleStatus(currentStatus);
        prevBleStatus = currentStatus;
    }

    // Print stream-ready transitions
    bool streamReady = isBleStreamReady(currentStatus);
    if (streamReady != wasStreamReady)
    {
        if (streamReady)
        {
            Serial.println("BLE serial stream connected and ready.");
            bleSerial.println("EQSP32 BLE serial connected.");
            bleSerial.println("Type in EQConnect BLE terminal or USB serial terminal to test the bridge.");
        }
        else
        {
            Serial.println("BLE serial stream not ready.");
        }

        wasStreamReady = streamReady;
    }

    // Handle BLE serial traffic
    handleBleSerialBridge(currentStatus);

    delay(10);
}