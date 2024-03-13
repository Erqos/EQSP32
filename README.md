# EQSP32 Library

The EQSP32 library provides a comprehensive suite of functionalities for the EQSP32,
an IIoT (Industrial Internet of Things) controller based on the ESP32S3.
This library enables easy interaction with the hardware's digital and analog pins,
power supply monitoring, buzzer control, serial communication, and cloud (database) variables,
facilitating the rapid development and deployment of IIoT solutions.

## Features

- **Configurable from EQConnect**: WiFi setup and other imporant high-level user parameters (like timezone, account email, etc.) may be configured from EQConnect mobile app.
- **IoT Handling**: Transparently synchronises and updates all cloud variables (user bool and user int, as defined in the library).
- **Pin Configuration**: Easily configure digital inputs, analog inputs, temperature inputs, PWM outputs, relays, and more.
- **Custom pin handling**: User may take control of any of the I/O and communication pins, to be used with third party or custom libraries.
- **Non-Blocking Dual-Core Code Execution**: Ensures that the EQ supervisor tasks are executed in a non-blocking manner, ideal for real-time applications.
- **Power Supply Monitoring**: Monitor input and output voltage levels in real-time.
- **Buzzer Control**: Activate a buzzer with a specified frequency.

## Installation

1. Download the latest release of the EQSP32 library from the GitHub repository.
2. In your Arduino IDE, go to Sketch > Include Library > Add .ZIP Library... and select the downloaded file.
3. Restart the Arduino IDE to see the library in the list of available libraries.

## Quick Start Guide

### Initialization

```cpp
#include "EQSP32.h"

EQSP32 eqsp32;

void setup()
{
	EQSP32Configs myEQSP32Configs;
	myEQSP32Configs.databaseURL = "Your firebase realtime database URL";
	myEQSP32Configs.databaseAPIKey = "Your firebase realtime database API key";
    myEQSP32Configs.devSystemID = "MysystemV1.0";		// This is a developer's parameter for the custom mobile app to connect only with compatible installations

    eqsp32.begin(myEQSP32Configs);
}

void loop()
{
    delay(1000);
}
