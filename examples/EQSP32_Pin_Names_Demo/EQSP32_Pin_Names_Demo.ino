/**
 * @file EQSP32_Pin_Names_Demo.ino
 * @brief Demonstrates pinName(): give the pins your own names, so EQMonitor shows "Feed pump" instead of "Pin 9".
 *
 * EQMonitor in the EQConnect app lists every terminal of the EQSP32 and of the expansion modules.
 * With pinName() the sketch gives a pin a name; EQMonitor shows it above the pin number.
 *
 * This example names:
 *   1. Four main-unit terminals: a start button (pin 1), a tank level sensor (pin 2),
 *      a feed pump (pin 9) and an alarm horn on a relay (pin 16)
 *   2. Pins of an EQXIO and an EQXRA module, also when no module is fitted
 *   3. One pin with a name in Greek, to show that any language works (UTF-8)
 * and shows two names that are refused, with the reason.
 *
 * The feed pump follows the start button, and the alarm horn sounds for 1 s every 10 s,
 * so EQMonitor has something to show next to the names.
 *
 * Hardware Setup:
 * - No wiring is needed to see the names in EQMonitor. To see the pins work as well:
 *   - Pin 1 (DIN): a start button.
 *   - Pin 2 (AIN): a tank level sensor with a voltage output.
 *   - Pin 9 (POUT): a feed pump.
 *   - Pin 16 (RELAY): an alarm horn.
 * - Optional: an EQXIO and an EQXRA on the expansion port. Their pins are named also when
 *   they are not fitted.
 *
 * Notes:
 * - Name the pins once, in setup(). Before or after begin() both work.
 * - Only physical pins take a name: the 16 terminals and the pins of a module as EQMonitor lists
 *   them (for example EQXHI pins 1-10, EQXRA relays 1-8 and analog outputs 9-10).
 *   Virtual channels, such as the EQXHI frequency channels, are refused.
 * - A name is at most 64 bytes of UTF-8 text (EQ_PIN_NAME_MAX_BYTES). English letters take
 *   1 byte each, Greek letters 2, Chinese characters 3. A longer name is refused, never cut.
 * - Empty text is refused. A pin without a pinName() call simply has no name.
 * - The name belongs to the pin, not to its mode: changing the mode keeps the name.
 * - A module pin can be named before the module is found, or while it is disconnected.
 * - Names are kept in RAM until the next restart; the sketch sets them again at start-up.
 * - EQMonitor reads the names when it connects. Names need EQConnect 2.1.0 or newer.
 * - pinName() returns true when the name is accepted and false otherwise; the previous name is then kept.
 * - Uses EQTimer for the alarm horn timing.
 *
 * @author Erqos Technologies
 * @date 2026-10-19
 */

#include <EQSP32.h>

EQSP32 eqsp32;
EQTimer hornCycle(10000);   // The alarm horn sounds once per cycle
EQTimer hornOn(1000);       // ...for this long

const int startButtonPin = EQ_PIN_1;
const int tankLevelPin   = EQ_PIN_2;
const int feedPumpPin    = EQ_PIN_9;
const int alarmHornPin   = EQ_PIN_16;

void nameOrReport(int pin, const char* name, const char* what);

void setup() {
    Serial.begin(115200);
    eqsp32.begin();

    Serial.println("\nEQSP32 Pin Names Demo\n");

    // 1. Main-unit terminals
    nameOrReport(startButtonPin, "Start button", "Pin 1");
    nameOrReport(tankLevelPin,   "Tank level",   "Pin 2");
    nameOrReport(feedPumpPin,    "Feed pump",    "Pin 9");
    nameOrReport(alarmHornPin,   "Alarm horn",   "Pin 16");

    // 2. Module pins, named with the same pin codes as pinMode() and readPin()
    nameOrReport(EQXIO(1, 3),          "Pump 1",       "EQXIO 1, pin 3");
    nameOrReport(EQXRA(1, EQXRA_RL_1), "Heater",       "EQXRA 1, relay 1");
    nameOrReport(EQXRA(1, EQXRA_AO_1), "Valve demand", "EQXRA 1, analog output 1");

    // 3. Any language: "Pump 2 - Outlet pressure" in Greek, 40 bytes
    nameOrReport(EQ_PIN_3, "Αντλία 2 - Πίεση εξόδου", "Pin 3");

    // Two names that are refused
    nameOrReport(EQXHI(1, EQXHI_FREQ_1), "Line frequency", "EQXHI 1, frequency channel 1 (virtual channel)");
    nameOrReport(EQ_PIN_4, "Boiler 2 - Return water temperature after the heat exchanger (C)", "Pin 4 (name of 64+ bytes)");

    // Pin modes, as in any sketch. The names do not depend on them.
    eqsp32.pinMode(startButtonPin, DIN);
    eqsp32.pinMode(tankLevelPin, AIN);
    eqsp32.pinMode(feedPumpPin, POUT);
    eqsp32.pinMode(alarmHornPin, RELAY);

    Serial.println("\nOpen EQMonitor in the EQConnect app to see the names.\n");

    hornCycle.start();
}

void loop() {
    // The feed pump runs while the start button is pressed
    eqsp32.pinValue(feedPumpPin, eqsp32.readPin(startButtonPin) ? 1000 : 0);

    // The alarm horn sounds for 1 s every 10 s
    if (hornCycle.isElapsed(true)) {
        eqsp32.pinValue(alarmHornPin, 1000);
        hornOn.start();
    }
    if (hornOn.isElapsed()) {
        eqsp32.pinValue(alarmHornPin, 0);
        hornOn.stop();
    }
}

void nameOrReport(int pin, const char* name, const char* what) {
    if (eqsp32.pinName(pin, name))
        Serial.printf("%-48s named \"%s\"\n", what, name);
    else
        Serial.printf("%-48s NOT named: \"%s\" was refused\n", what, name);
}
