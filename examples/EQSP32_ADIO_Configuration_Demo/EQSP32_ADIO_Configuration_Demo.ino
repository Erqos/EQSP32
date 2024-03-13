#include "EQSP32.h"

EQSP32 eqsp32;

void setup() {

    // EQSP32 system configuration
    EQSP32Configs myEQSP32Configs;
    myEQSP32Configs.databaseURL = "Your firebase realtime database URL";
    myEQSP32Configs.databaseAPIKey = "Your firebase realtime database API key";
    myEQSP32Configs.devSystemID = "ADIODemoV1.0";

    eqsp32.begin(myEQSP32Configs, true);		// Verbose enabled

    Serial.begin(115200);   // For debug purposes

    // Initialize ADIO in various input modes
    eqsp32.pinMode(1, DIN); // Configure pin 1 as Digital Input

    eqsp32.pinMode(2, AIN); // Configure pin 2 as Analog Input

    eqsp32.pinMode(3, SWT); // Configure pin 3 as Switch with 150 ms debounce
    eqsp32.configSWT(3, 150);

    eqsp32.pinMode(4, TIN); // Configure pin 4 as Temperature Input
    eqsp32.configTIN(4, 3988, 10000); // Set NTC parameters for pin 4, beta 3988 and ref Resistance 10K

    // Initialize ADIO in various output modes
    eqsp32.pinMode(5, POUT); // Configure pin 5 as Power PWM Output

    eqsp32.pinMode(6, RELAY); // Configure pin 6 as Relay
    eqsp32.configRELAY(6, 350, 1500); // Set relay holding power to 35% max after 1500ms from initial power
    
    // Set initial states or values for some pins
    eqsp32.pinValue(5, 500); // Set pin 5 (POUT) to 50% PWM
    eqsp32.pinValue(6, 0); // Turn off relay connected to pin 6
}

void loop() {
    // Simple demo operations
    int digitalState = eqsp32.readPin(1); // Read digital state from pin 1 (DIN)
    int analogValue = eqsp32.readPin(2); // Read analog value from pin 2 (AIN)
    int switchState = eqsp32.readPin(3); // Read digital state from pin 3 (SWT)
    int temperature = eqsp32.readPin(4); // Read temperature from pin 4 (TIN)

    // Toggle relay based on digital input
    if (digitalState == HIGH) {
        eqsp32.pinValue(6, 1000); // Turn on relay if digital input is HIGH
    } else {
        eqsp32.pinValue(6, 0); // Turn off relay if digital input is LOW
    }

    // Adjust PWM based on analog input only if switch at pin 3 is HIGH
    if (switchState) {
        int pwmValue = map(analogValue, 0, 5000, 0, 1000); // Map analog value (0mV-5000mV) to PWM range 0 to 1000 for 0% to 100%
        eqsp32.pinValue(5, pwmValue); // Set PWM output based on analog input
    }
    else {
        eqsp32.pinValue(5, 0); // Set PWM output based on analog input
    }

    delay(100); // Delay for stability and to reduce loop speed
}
