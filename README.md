# EQSP32 Library

The EQSP32 library provides a comprehensive suite of functionalities for the EQSP32,
an IIoT (Industrial Internet of Things) controller based on the ESP32S3.
This library enables easy interaction with the hardware's digital and analog pins,
power supply monitoring, buzzer control, serial communication, and MQTT integrations like Home Assistant, Node-RED, etc.,
facilitating the rapid development and deployment of IIoT solutions.

## Features

- **Configurable from EQConnect**: WiFi (provisioning) setup and other imporant high-level user parameters (like timezone, account email, etc.) may be configured from EQConnect mobile app.
- **MQTT**: Transparently creates interfacing entities for integrating EQSP32 with MQTT solutions like Home Assistant, Node-RED, etc..
- **Pin Configuration**: Easily configure digital inputs, analog inputs, temperature inputs, PWM outputs, relays, and more.
- **Custom pin handling**: User may take control of any of the I/O and communication pins, to be used with third party or custom libraries.
- **Non-Blocking Dual-Core Code Execution**: Ensures that the EQ supervisor tasks are executed in a non-blocking manner, ideal for real-time applications.
- **Power Supply Monitoring**: Monitor input and output voltage levels in real-time.
- **Buzzer Control**: Activate a buzzer with a specified frequency.

## Installation

The EQSP32 library needs the **RadioLib** library, version **7.7.x**. Without RadioLib, the build stops with `RadioLib.h: No such file or directory`.

The EQSP32 library is built with RadioLib 7.7.1. Keep RadioLib at 7.7.x: with another version (7.8 or newer) the onboard LoRa radio may not work. Programs that do not use LoRa are not affected.

### From the Arduino IDE Library Manager

1. In your Arduino IDE, go to Sketch > Include Library > Manage Libraries... and search for **EQSP32**.
2. Click Install. When the IDE asks to install the missing dependencies, click **Install all**. This installs RadioLib 7.7.x with the EQSP32 library.

### From a .ZIP file

1. Download the latest release of the EQSP32 library from the GitHub repository.
2. In your Arduino IDE, go to Sketch > Include Library > Add .ZIP Library... and select the downloaded file.
3. A .ZIP install does not add RadioLib. Go to Sketch > Include Library > Manage Libraries..., search for **RadioLib**, select version **7.7.1** and click Install.
4. Restart the Arduino IDE to see the library in the list of available libraries.

## Quick Start Guide

### Initialization

```cpp
#include "EQSP32.h"

EQSP32 eqsp32;

void setup()
{
    	eqsp32.begin(true);	// Enable system messages
}

void loop()
{
    	delay(1000);
}
