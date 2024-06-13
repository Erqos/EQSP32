#include "EQSP32.h"

/* 
  In this example we demonstrate how to create interfaces to Home Assistant.

  We setup pins in different modes of operation and then we create interfaces for them to be monitored in
  Home Assistant as an MQTT device.
  Each ADIO pin interface needs a name and the assigned pin. The ADIO values are then automatically updated in
  Home Assistant.
*/

EQSP32 eqsp32;

void setup() {
    EQSP32Configs configs;
    configs.devSystemID = "SYSTEM_ID";                       // Needed for custom mobile app and for MQTT

    eqsp32.begin(configs, true);

    // Configure various pin modes
    eqsp32.pinMode(1, DIN);    // Digital Input
    eqsp32.pinMode(2, AIN);    // Analog Input (pins 1-8 support AIN)
    eqsp32.pinMode(3, SWT);    // Switch with debounce, default debounce for SWT
    eqsp32.pinMode(4, RAIN);   // Relative analog input mode (pins 1-8 support RAIN)
    eqsp32.pinMode(5, RELAY);  // Relay, default configs for relay

    eqsp32.pinMode(6, POUT);   // Power PWM Output
    eqsp32.configPOUTFreq(3000);  // Set PWM frequency to 3000 Hz

    eqsp32.pinMode(7, TIN);    // Temperature Input (pins 1-8 support TIN)
    eqsp32.configTIN(7, 3945, 10000);  // Configure NTC parameters for pin 5

    // Create interfaces for each pin
    createInterface("Digital Input", 1);        // Directly updates from pin value
    createInterface("Analog Input", 2);         // Directly updates from pin value
    createInterface("Switch", 3);               // Directly updates from pin value
    createInterface("Relative Analog Input", 4); // Directly updates from pin value
    createInterface("Relay", 5);                // Directly updates from pin value
    createInterface("Power PWM Output", 6);     // Directly updates from pin value
    createInterface("Temperature Input", 7);    // Directly updates from pin value    

    // Set initial values for output pins
    eqsp32.pinValue(6, 500);   // Set pin 3 (POUT) to 50% PWM
    eqsp32.pinValue(7, 1000);  // Set pin 7 (RELAY) to start at 100% PWM
}

void loop() {
    delay(1000); // Pins assigned to interfaces automatically update their values, nothing for us to do in the loop
}
