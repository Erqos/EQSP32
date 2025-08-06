/**
 * @file EQSP32_Pulse_Counting_PCC_Mode_Demo.ino
 * @brief Demonstrates the use of PCC mode on EQSP32 for pulse counting and measurement.
 *
 * This example shows four PCC use cases using terminal pins 9–16:
 *   1. Count rising edge pulses
 *   2. Count both edges (toggle mode)
 *   3. Accumulate total pulse count (e.g. simulate liters from a flow meter)
 *   4. Calculate pulse frequency (Hz) over time using EQTimer
 *
 * Notes:
 * - Only EQSP32 terminal pins 9–16 support PCC mode.
 * - A maximum of 4 PCC pins can be active at once.
 * - The count is automatically cleared upon each readPin() call.
 * - Uses EQTimer to ensure time measurements are accurate even with loop() delays.
 */

#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer freqTimer;

const int risingPin     = EQ_PIN_9;
const int togglePin     = EQ_PIN_10;
const int accumPin      = EQ_PIN_11;
const int freqPin       = EQ_PIN_12;

unsigned long accumulatedPulses = 0;

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
    eqsp32.configPCC(accumPin, ON_RISING);     // Accumulating rising pulses
    eqsp32.configPCC(freqPin, ON_RISING);      // Used for pulse frequency (Hz) calculation

    freqTimer.start(); // Start timer to measure elapsed time
}

void loop() {
    printRisingPulses();
    printTogglePulses();
    printAccumulatedPulses();
    printCalculatedPulseFreq();

    Serial.println("\n-----------------------------\n");
    delay(1000);
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
    int count = eqsp32.readPin(accumPin);
    accumulatedPulses += count;

    // Simulate conversion (e.g. 450 pulses = 1L)
    float liters = accumulatedPulses / 450.0;

    Serial.print("Accumulated pulses (Pin ");
    Serial.print(accumPin);
    Serial.print("): ");
    Serial.print(accumulatedPulses);
    Serial.print("  ->  Approx. ");
    Serial.print(liters, 3);
    Serial.println(" L");
}

void printCalculatedPulseFreq() {
    int pulses = eqsp32.readPin(freqPin);
    unsigned long elapsedMs = freqTimer.value();

    float frequency = 0;
    if (elapsedMs > 0) {
        frequency = (pulses * 1000.0) / elapsedMs;
    }

    Serial.print("Pulse frequency (Pin ");
    Serial.print(freqPin);
    Serial.print("): ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");

    freqTimer.reset(); // Restart timing for next interval
}