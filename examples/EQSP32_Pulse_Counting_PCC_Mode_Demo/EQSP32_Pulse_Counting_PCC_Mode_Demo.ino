/**
 * @file EQSP32_Pulse_Counting_PCC_Mode_Demo.ino
 * @brief Demonstrates the use of PCC mode on EQSP32 for pulse counting and measurement.
 *
 * This example shows four PCC use cases using terminal pins 9–16:
 *   1. Count rising edge pulses
 *   2. Count both edges (toggle mode)
 *   3. Keep a running total in the library (e.g. simulate liters from a flow meter), with zero and preset
 *   4. Calculate pulse frequency (Hz) over time using EQTimer
 *
 * Type 'z' in the Serial Monitor to zero the running total, or 'p' to preset it to 1 liter.
 *
 * Notes:
 * - Only EQSP32 terminal pins 9–16 support PCC mode.
 * - A maximum of 4 PCC pins can be active at once.
 * - By default the count is automatically cleared upon each readPin() call (cases 1, 2 and 4).
 *   For a total of any size with this default, add up the readings in your sketch.
 * - With configPCC(pin, edge, false) the library keeps a running total that reading does not clear (case 3).
 *   pinValue() zeroes or presets it. It counts up to 2,147,483,647 and then restarts from 0.
 * - Uses EQTimer to ensure time measurements are accurate even with loop() delays.
 *
 * @author Erqos Technologies
 * @date 2026-10-19
 */

#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer freqTimer;

const int risingPin     = EQ_PIN_9;
const int togglePin     = EQ_PIN_10;
const int accumPin      = EQ_PIN_11;
const int freqPin       = EQ_PIN_12;

const int pulsesPerLiter = 450;     // Simulated flow meter: 450 pulses = 1L

void handleSerialCommands();
void printRisingPulses();
void printTogglePulses();
void printAccumulatedPulses();
void printCalculatedPulseFreq();

void setup() {
    Serial.begin(115200);
    eqsp32.begin();

    Serial.println("\nEQSP32 PCC Pulse Count Demo\n");

    // Set pins to PCC mode first
    eqsp32.pinMode(risingPin, PCC);
    eqsp32.pinMode(togglePin, PCC);
    eqsp32.pinMode(accumPin, PCC);
    eqsp32.pinMode(freqPin, PCC);

    // Then configure pulse edge behavior
    eqsp32.configPCC(risingPin, ON_RISING);    // Count only rising edges
    eqsp32.configPCC(togglePin, ON_TOGGLE);    // Count both rising and falling edges
    eqsp32.configPCC(accumPin, ON_RISING, false);   // Running total of rising pulses: reading does not clear it
    eqsp32.configPCC(freqPin, ON_RISING);      // Used for pulse frequency (Hz) calculation

    freqTimer.start(); // Start timer to measure elapsed time
}

void loop() {
    handleSerialCommands();

    printRisingPulses();
    printTogglePulses();
    printAccumulatedPulses();
    printCalculatedPulseFreq();

    Serial.println("\n-----------------------------\n");
    delay(1000);
}

void handleSerialCommands() {
    while (Serial.available()) {
        char command = Serial.read();
        if (command == 'z') {
            eqsp32.pinValue(accumPin, 0);                   // Zero the running total
            Serial.println("Running total zeroed");
        } else if (command == 'p') {
            eqsp32.pinValue(accumPin, pulsesPerLiter);      // Preset the running total, here to 1 liter
            Serial.println("Running total preset to 1 L");
        }
    }
}

void printRisingPulses() {
    int count = eqsp32.readPin(risingPin);
    Serial.print("Rising pulses (Pin ");
    Serial.print(risingPin);
    Serial.print("): ");
    Serial.println(count);
}

void printTogglePulses() {
    int count = eqsp32.readPin(togglePin);
    Serial.print("Toggle pulses (Pin ");
    Serial.print(togglePin);
    Serial.print("): ");
    Serial.println(count);
}

void printAccumulatedPulses() {
    int totalPulses = eqsp32.readPin(accumPin);     // Running total kept by the library, not cleared by reading

    // Simulate conversion (e.g. 450 pulses = 1L)
    float liters = (float)totalPulses / pulsesPerLiter;

    Serial.print("Accumulated pulses (Pin ");
    Serial.print(accumPin);
    Serial.print("): ");
    Serial.print(totalPulses);
    Serial.print("  ->  Approx. ");
    Serial.print(liters, 3);
    Serial.println(" L");
}

void printCalculatedPulseFreq() {
    int pulses = eqsp32.readPin(freqPin);
    unsigned long elapsedMs = freqTimer.value();
    freqTimer.reset(); // Restart timing for next interval, right after the reading it belongs to

    float frequency = 0;
    if (elapsedMs > 0) {
        frequency = (pulses * 1000.0) / elapsedMs;
    }

    Serial.print("Pulse frequency (Pin ");
    Serial.print(freqPin);
    Serial.print("): ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");
}