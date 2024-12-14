/**
 * @file EQSP32_Onboard_Buzzer_Demo.ino
 * @brief Demonstrates the usage of the onboard buzzer on the EQSP32 module, 
 * including tones with specified durations and indefinite tones requiring manual stopping.
 *
 * This example shows how to use the `buzzerOn()` function to activate the onboard buzzer 
 * with different frequencies. It highlights:
 * - Using `buzzerOn()` with a specified duration to play tones for a fixed period.
 * - Using `buzzerOn()` without a duration to play an indefinite tone.
 * - Stopping the fixed period or indefinite tones manually with `buzzerOff()`.
 *
 * Features:
 * - Activates the buzzer at different frequencies and durations.
 * - Demonstrates indefinite tone playback until manually stopped.
 * - Demonstrates interrupting a fixed-duration tone.
 * - Uses serial messages to guide the user through the demo.
 *
 * Buzzer Function Summary:
 * - `buzzerOn(freq, duration_ms)`:
 * - Activates the buzzer at a specified frequency in Hz.
 * - Plays for `duration_ms` milliseconds if specified, or indefinitely if not.
 * - New `buzzerOn()` calls or `buzzerOff()` interrupt the current tone.
 * - `buzzerOff()` stops the buzzer immediately.
 */

#include <EQSP32.h>  // Include the EQSP32 library

// Create an instance of the EQSP32 library
EQSP32 eqsp32;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);

    Serial.println("\nStarting EQSP32 Buzzer Demo...");
    Serial.println("Demonstrating tones with and without durations.");

    // Initialize the EQSP32 module
    eqsp32.begin();


    delay(1000);

        // Tone 1: Fixed duration, but let it complete
    Serial.println("\nPlaying tone: 440 Hz for 1000 ms...");
    eqsp32.buzzerOn(440, 1000);  // Play a 440 Hz tone for 1 second
    delay(2000);                 // Wait 2 seconds to show it stops automatically

    // Tone 2: Indefinite duration, stop manually
    Serial.println("\nPlaying tone: 220 Hz indefinitely...");
    eqsp32.buzzerOn(220);  // Play a 220 Hz tone indefinitely
    delay(2000);           // Wait 2 seconds to demonstrate the indefinite tone
    Serial.println("Stopping indefinite tone after 2000 ms...");
    eqsp32.buzzerOff();    // Manually stop the buzzer
    delay(2000);           // Wait before the next tone

    // Tone 3: Fixed duration, but stop manually before completion
    Serial.println("\nPlaying tone: 880 Hz for 5000 ms...");
    eqsp32.buzzerOn(880, 5000);  // Play an 880 Hz tone for 5 seconds
    delay(2000);                 // Interrupt after 2 seconds
    Serial.println("Interrupting fixed-duration tone after 2000 ms...");
    eqsp32.buzzerOff();          // Manually stop the buzzer

    Serial.println("Press reset to play the demo again.");
}

void loop() {
    delay(1000);
}
