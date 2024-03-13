#include "EQSP32.h"

/* 
  In this example we assume that a relay is connected on pin 9, to control a pump
  a momentary button is connected as debounced input at pin 1, to control the pump
  and cloud user bool variable 1 is also used to remotely control the pump.

  Each time the button is pressed, we want to change the state of the relay. So we will read for a rising event on pin 1.
  For every ON_RISING event we get, we will check the current value of our relay. If it is off we will turn it on and vice versa.
  For our IoT part to be updated, we will also update the user bool var 1 about the state of our relay.
  Then we will check the user bool. If it is HIGH we enable the relay, if not, we disable it.
  NOTE: The user bool var is automatically syncronized with the database, thus is may be triggered outside our loop() funciton.
*/

EQSP32 eqsp32;

void setup() {
    EQSP32Configs myEQSP32Configs;
    myEQSP32Configs.databaseURL = "Your firebase realtime database URL";
    myEQSP32Configs.databaseAPIKey = "Your firebase realtime database API key";
    myEQSP32Configs.devSystemID = "CloudTest";

    eqsp32.begin(myEQSP32Configs, true);		// Verbose enabled

    // Configure the switch with debounce
    eqsp32.pinMode(1, SWT);       // Assuming pin 1 is a switch input
    eqsp32.configSWT(1, 150);     // Set debounce 150 ms for the switch

    // Configure the relay with specific parameters
    eqsp32.pinMode(9, RELAY);         // Assuming pin 9 controls a pump relay
    eqsp32.configRELAY(9, 400, 1000); // derates to 40% after 1 second
}

void loop() {
  // put your main code here, to run repeatedly:
	if (eqsp32.readPin(1, ON_RISING))		// When switch is pressed
	{
		if (eqsp32.readPin(9) != 0)		    // Read the relay's power, if the relay is not off
		{
			eqsp32.pinValue(9, 0);          // Turn relay off
			eqsp32.writeUserBool(1, false);		// Update cloud bool variable to be false (this will update the database and the change will appear in the custom mobile app)
		}
		else
		{
			eqsp32.pinValue(9, 1000);       // Turn relay on at 100%
			eqsp32.writeUserBool(1, true);		// Update cloud bool variable state to be true, meaning that the relay is on
		}
	}

  if (eqsp32.readUserBool(1))		// If the cloud variable is set to true (HIGH) either by the program or by the mobile app, we turn on the relay
    eqsp32.pinValue(9, 1000);
  else
    eqsp32.pinValue(9, 0);

  delay(10);
}
