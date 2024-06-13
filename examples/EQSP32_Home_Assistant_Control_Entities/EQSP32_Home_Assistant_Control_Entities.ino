#include "EQSP32.h"

/* 
  In this example we will demonstrate how to create custom interfaces to interact with Home Assistant.
  The custom interfaces might any of the sensor interfaces:
  WATER_CONSUMPTION_SENSOR, VOLUME_STORAGE_SENSOR, VOLUME_FLOW_RATE_SENSOR, PRECIPITATION_INTENSITY_SENSOR
  Any of the control interfaces:
  SWITCH, BUTTON, VALVE, INTEGER_VALUE, FLOAT_VALUE
  And any custom sensor or binary sensor is supported by the MQTT integration in Home Assistnat (also mentioned in EQSP32.h)

  We create two "button" or BUTTON interfaces and one "switch" or SWITCH. These will appear in home assistant and we will be able to accept commands.
  For the SWITCH interface we may also update its state after processing the command, so there is feedback to home assistant.

  For the most advanced approach we may create custom sensors and binary sensors, which we will update with our measurements.
  These will be also be available in home assistant.

  All control interfaces can also be written. This means that for every BUTTON or SWITCH, we can generate the commands ourselves by using
  the writeInterfaceSTRING function.
*/

EQSP32 eqsp32;

#define LIGHTS  1

void setup() {
    EQSP32Configs configs;
    configs.devSystemID = "SYSTEM_ID";                       // Needed for custom mobile app and for MQTT

    eqsp32.begin(configs, true);

    eqsp32.pinMode(LIGHTS, RELAY);

    // Create interfaces for each pin
    createInterface("Open Door", "button");           // Custom interface, "button" interfaces are not updated, they only generate PRESS requests to be handled
    createInterface("Close Door", "button");          // Custom interface, "button" interfaces are not updated, they only generate PRESS requests to be handled
    createInterface("Garden Lights", "switch");       // Custom interface, the switch state should be updated in the loop

    // Advanced funcitonality
    // Creating custom sensor
    SensorEntity myTempSensor;
    myTempSensor.devClass = "temperature";
    myTempSensor.unit_of_measurement = "°C";
    createInterface("Temperature Sensor", myTempSensor);  // This custom interface is a sensor and needs to be updated in the loop 

    // Creating custom binary sensor
    BinarySensorEntity myMotionSensor;
    myMotionSensor.devClass = "motion";
    createInterface("Motion Sensor", myMotionSensor);     // This custom interface is a binary sensor and needs to be updated in the loop

    // Creating custom interface to send commands
    createInterface("Button Command", "button");
}

void loop() {
  if (readInterfaceBOOL("Garden Lights"))
    eqsp32.pinValue(LIGHTS, 1000);      // Turn ON the relay controlling the lights
  else
    eqsp32.pinValue(LIGHTS, 0);         // Turn OFF the relay controlling the lights
  updateInterfaceBOOL("Garden Lights", (bool)eqsp32.readPin(LIGHTS));   // Update the state of the switch based on the state of the relay

  if (readInterfaceBOOL("Open Door")) // The button interface is cleared once read, the next read will return false unless a new press request has been received
  {
    // Open the door
  }
  // "button" interfaces do not update

  if (readInterfaceBOOL("Close Door")) // The button interface is cleared once read, the next read will return false unless a new press request has been received
  {
    // Close the door
  }
  // "button" interfaces do not update

  int temperature = 25;
  bool motionSense = false;

  // read your sensors

  updateInterfaceINT("Temperature Sensor", temperature);    // update sensor in home assistant
  updateInterfaceBOOL("Motion Sensor", motionSense);        // update binary sensor in home assistant

  // if (some condition here)
  //  writeInterfaceSTRING("Button Command", "PRESS");    // This would trigger a button press command to all other EQSP32 devices with the same interface (including this device)


  delay(100); // Pins assigned to interfaces automatically update their values, nothing for us to do in the loop
}
