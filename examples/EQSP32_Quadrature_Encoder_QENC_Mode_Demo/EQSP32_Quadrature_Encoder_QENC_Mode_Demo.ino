/**
 * @file EQSP32_Quadrature_Encoder_QENC_Mode_Demo.ino
 * @brief Demonstrates the use of QENC mode on EQSP32 to read the position and the speed of quadrature encoders.
 *
 * A quadrature (incremental) encoder has two signals, A and B, a quarter of a period apart.
 * EQSP32 counts their edges in hardware and keeps a signed position: it rises when the encoder
 * turns one way and falls when it turns the other way.
 *
 * This example uses two encoders:
 *   1. A motor encoder on pins 9 (A) and 10 (B): position in counts and turns, speed in counts/s and RPM
 *   2. A handwheel on pins 11 (A) and 12 (B): one count per signal period, counting direction reversed
 *
 * Type 'z' in the Serial Monitor to zero the position of the motor encoder.
 *
 * Hardware Setup:
 * - Motor encoder: signal A to terminal 9, signal B to terminal 10.
 * - Handwheel: signal A to terminal 11, signal B to terminal 12.
 * - Connect the encoder ground to the EQSP32 GND, and supply the encoder as its datasheet requires.
 * - Power the EQSP32 from VIn. On USB power alone the first counts after a pause can be lost.
 *
 * Notes:
 * - Set QENC on pin 9, 11, 13 or 15 (signal A). The next pin (signal B) is taken automatically.
 * - Up to 4 encoders can be active at once. The four hardware counters are shared with PCC mode.
 * - readPin() returns the position from either pin of the pair. Reading does not clear it.
 * - The position is signed, so -1 is a valid position in this mode and not an error.
 * - pinValue() sets the position: 0 zeroes it, any other value presets it.
 * - Uses EQTimer for the print interval and for the time between two position readings.
 *
 * @author Erqos Technologies
 * @date 2026-10-19
 */

#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer printTimer(500);    // Print interval in ms
EQTimer speedTimer;         // Time between two position readings

const int motorEncoderPin   = EQ_PIN_9;     // Signal A on pin 9, signal B on pin 10
const int handwheelPin      = EQ_PIN_11;    // Signal A on pin 11, signal B on pin 12

const int motorEncoderPPR   = 600;                      // Pulses per revolution, from the encoder's datasheet
const int motorCountsPerRev = motorEncoderPPR * 4;      // QENC_X4 gives four counts per pulse

int previousPosition = 0;

void zeroMotorEncoderOnRequest();
void printMotorEncoder();
void printHandwheel();

void setup() {
    Serial.begin(115200);
    eqsp32.begin();

    Serial.println("\nEQSP32 QENC Quadrature Encoder Demo\n");

    // Set the first pin of each pair to QENC mode; the second pin of the pair follows
    if (!eqsp32.pinMode(motorEncoderPin, QENC))
        Serial.println("Motor encoder: QENC mode was not accepted");
    if (!eqsp32.pinMode(handwheelPin, QENC))
        Serial.println("Handwheel: QENC mode was not accepted");

    // Then configure the resolution (counts per signal period) and the counting direction.
    // The resolution can be given by name or by number: QENC_X4 is 4, QENC_X2 is 2 and QENC_X1 is 1.
    if (!eqsp32.configQENC(motorEncoderPin, QENC_X4))   // By name: every edge of A and B counts (the default)
        Serial.println("Motor encoder: configuration was not accepted");
    if (!eqsp32.configQENC(handwheelPin, 1, true))      // By number: one count per period, direction reversed
        Serial.println("Handwheel: configuration was not accepted");

    // Start both positions from zero
    eqsp32.pinValue(motorEncoderPin, 0);
    eqsp32.pinValue(handwheelPin, 0);

    printTimer.start();
    speedTimer.start();     // Start timer to measure the time between readings
}

void loop() {
    zeroMotorEncoderOnRequest();

    if (printTimer.isElapsed(true)) {       // Runs once per print interval
        printMotorEncoder();
        printHandwheel();
        Serial.println("\n-----------------------------\n");
    }
}

void zeroMotorEncoderOnRequest() {
    while (Serial.available()) {
        if (Serial.read() == 'z') {
            eqsp32.pinValue(motorEncoderPin, 0);    // Zero the position; a value other than 0 presets it
            previousPosition = 0;                   // Keep the next speed calculation correct
            Serial.println("Motor encoder position zeroed");
        }
    }
}

void printMotorEncoder() {
    int position = eqsp32.readPin(motorEncoderPin);     // Signed position in counts, not cleared by reading
    unsigned long elapsedMs = speedTimer.value();
    speedTimer.reset();     // Restart timing for next interval

    // Speed from two readings: the counts since the previous reading over the time between them
    float countsPerSecond = 0;
    if (elapsedMs > 0) {
        countsPerSecond = ((position - previousPosition) * 1000.0) / elapsedMs;
    }
    previousPosition = position;

    float turns = (float)position / motorCountsPerRev;
    float rpm = (countsPerSecond * 60.0) / motorCountsPerRev;

    Serial.print("Motor encoder (Pins ");
    Serial.print(motorEncoderPin);
    Serial.print("/");
    Serial.print(motorEncoderPin + 1);
    Serial.print("): ");
    Serial.print(position);
    Serial.print(" counts  ->  ");
    Serial.print(turns, 3);
    Serial.print(" turns, ");
    Serial.print(countsPerSecond, 1);
    Serial.print(" counts/s, ");
    Serial.print(rpm, 1);
    Serial.println(" RPM");
}

void printHandwheel() {
    int clicks = eqsp32.readPin(handwheelPin + 1);      // The second pin of the pair returns the same position

    Serial.print("Handwheel (Pins ");
    Serial.print(handwheelPin);
    Serial.print("/");
    Serial.print(handwheelPin + 1);
    Serial.print("): ");
    Serial.print(clicks);
    Serial.println(" counts");
}
