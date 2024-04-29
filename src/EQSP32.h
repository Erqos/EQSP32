#ifndef EQSP32_h
#define EQSP32_h

#include <Arduino.h>
#include "time.h"

#include "driver/ledc.h"
#include "driver/mcpwm.h"
#include "driver/twai.h"

#define POUT_PATCH(eq, pin, mode)    do {int native = eq.getPin(pin); eq.pinMode(pin, mode); pinMode(native, OUTPUT);} while (0);

// User available pin codes
#define EQ_PIN_1        1
#define EQ_PIN_3        3
#define EQ_PIN_2        2
#define EQ_PIN_4        4
#define EQ_PIN_5        5
#define EQ_PIN_6        6
#define EQ_PIN_7        7
#define EQ_PIN_8        8
#define EQ_PIN_9        9
#define EQ_PIN_10       10
#define EQ_PIN_11       11
#define EQ_PIN_12       12
#define EQ_PIN_13       13
#define EQ_PIN_14       14
#define EQ_PIN_15       15
#define EQ_PIN_16       16

#define EQ_AO_1         1
#define EQ_AO_2         2

// User available peripheral pin codes
#define EQ_RS232_TX     20
#define EQ_RS232_RX     21
#define EQ_RS485_TX     22
#define EQ_RS485_RX     23
#define EQ_RS485_EN     24
#define EQ_CAN_TX       25
#define EQ_CAN_RX       26

#define IS_ADIO_PIN(p)      (p >= EQ_PIN_1 && p <= EQ_PIN_16 ? true : false)
#define IS_RS232_PIN(p)     (p == EQ_RS232_TX || p == EQ_RS232_RX ? true : false)
#define IS_RS485_PIN(p)     (p >= EQ_RS485_TX && p <= EQ_RS485_EN ? true : false)
#define IS_CAN_PIN(p)       (p == EQ_CAN_TX || p == EQ_CAN_RX ? true : false)


// EQSP32 pin modes
enum PinMode : uint8_t {
    NO_MODE = 0xFF,
    CUSTOM  = 0xFE,
    DIN     = 0,
    DOUT,               // Not used
    AIN,
    AOUT,
    POUT,
        // Special modes
    SWT     = 8,        // Special DIN mode with debouncing timer
    TIN,                // Special AIN mode, automatic temperature convertion
    RELAY,              // Special POUT mode, starts with set power and after set time drops to holding power
    RAIN,               // Relative analog input, this returns a value of 0-1000 representing the % of read value versus the VOut reference voltage
};

enum TrigMode {
    STATE,
    ON_RISING,
    ON_FALLING,
    ON_TOGGLE
};

// CAN Bus
#define CanMessage      twai_message_t

enum CanBitRates
{
    CAN_25K = 0,    // Enumerator for 25 kbps
    CAN_50K,        // Enumerator for 50 kbps
    CAN_100K,       // Enumerator for 100 kbps
    CAN_125K,       // Enumerator for 125 kbps
    CAN_250K,       // Enumerator for 250 kbps
    CAN_500K,       // Enumerator for 500 kbps
    CAN_800K,       // Enumerator for 800 kbps
    CAN_1000K       // Enumerator for 1000 kbps (1 Mbps)
};


enum EQSerialMode {
    RS232,
    RS232_INV,
    RS485_TX,
    RS485_RX,
};


typedef struct
{
    std::string databaseURL;
    std::string databaseAPIKey;
    std::string devSystemID;        // Developer system ID (assigned by the system's developer, hardcoded by developer, READ ONLY access by external app)
    std::string userDevName;        // Device name (assigned by the end user, default value assigned on first flash, user has READ/WRITE access)
    bool relaySequencer = true;
} EQSP32Configs;


class EQ_Private;       // Forward declaration of the nested private class

class EQSP32 {
public:
    EQSP32();
    ~EQSP32();

        // Main EQ initialization
    /**
     * @brief Initializes the EQSP32 module.
     * 
     * This function MUST be called at the beginning of your application for the EQSP32 module to run properly.
     * The initialization includes:
     * 
     * - Setting up peripheral communication.
     * - Initializing WiFi and BLE modules.
     * - Configuring all pins in safe Digital Input (DIN) state.
     * - Initializing RS232 serial communication, default 115200 baud.
     * - Initiating the EQSP32 System Manager Task for continuous system management.
     * - Starting database-related tasks for data storage and retrieval.
     * 
     * @attention Create a single EQSP32 eqsp32; object and use this for all operations to avoid conflicts.
     * 
     * @note It is CRITICAL to call this function at the beginning of your EQSP32 application for proper operation.
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * eqsp32.begin();
     */
    void begin(EQSP32Configs eq_configs = {}, bool verboseEnabled = false);
    void begin(bool verboseEnabled);
    
    // Only for self-testing, not to be used by the user
    void begin(std::string command);

    /**
     * @brief Retrieves the corresponding ESP32 pin number for a given EQSP32 pin index.
     * 
     * This function maps a high-level EQSP32 pin index to the actual ESP32 GPIO number or peripheral pin.
     * It supports mapping for ADIO pins, RS232, RS485, and CAN bus pins. For ADIO pins, it sets the pin mode to CUSTOM.
     * For serial communication pins (RS232 and RS485), it configures the hardware appropriately to support the respective RS232 or RS485 protocol.
     * For CAN pins, it returns the native ESP32 pin mapped on the CAN driver and deinitializes the CAN peripheral. It is up to the user to initialize the CAN peripheral based on their needs.
     * 
     * @param pinIndex The index of the EQSP32 pin (e.g., EQ_RS232_TX, EQ_RS485_RX).
     * @return The corresponding ESP32 pin number if the pin is valid, or -1 if the pin index does not match any known configuration.
     * 
     * @example
     * int rs232Tx = eqsp32.getPin(EQ_RS232_TX); // Gets the ESP32 pin number for RS232 transmission.
     * int rs232Rx = eqsp32.getPin(EQ_RS232_RX); // Gets the ESP32 pin number for RS232 transmission.
     */
    int getPin(int pinIndex);

        // EQ pin configurations
    /**
     * @brief Sets the mode for a specified pin on the EQSP32.
     * 
     * This function allows you to set the mode for a pin on the EQSP32,
     * which has 16 analog and digital I/O pins.
     * Pins 1-8 also support both analog input, while pins 9-16 also support pseudo-analog output (pseudo-analog using PWM).
     * The pin can be set to different modes, including:
     * 
     * - Digital Input (DIN): Configures the pin as a standard digital input.
     * - Analog Input (AIN): Configures the pin as an analog input for pins 1-8.
     * - Pseudo-Analog Output (AOUT): Configures the pin as an Pseudo-Analog output for pins 9-16. You can specify the PWM frequency using the 'freq' parameter. The duty will be set using the 'pinValue' function.
     * - Power PWM Output (POUT): Configures the pin as a power PWM output. The duty will be set using the 'pinValue' function.
     *      //Special Modes
     * - Switch (SWT): Configures the pin as a special digital input mode with debouncing timer.
     * - Temperature Input (TIN): Configures the pin as a special analog input mode for automatic temperature conversion in celsius.
     * - Relay (RELAY): Configures the pin as a special power PWM output mode for relay control. Starts with set power from 'pinValue', and after a set time drops to holding power. Needs to be set to 0 before restarting the start-hold power cycle.
     * - Relative AIN (RAIN): Pin operates in AIN mode and returns the measured voltage relative to 5V Vout in %, 1000 represents 100%.
     * 
     * The function returns true if the configuration is successful, and false otherwise.
     * 
     * @param pinIndex The index of the pin to set the mode for (1 to 16).
     * @param mode The mode to set for the pin. Available options are DIN, DOUT, AIN, AOUT, POUT, SWT, TIN, RELAY.
     * @param freq The PWM frequency to use for analog output (AOUT) mode. Ignored for other modes. Default is 500 Hz.
     * 
     * @return true if the configuration is successful, false otherwise.
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::pinMode(3, DOUT); // Configures pin 3 as a digital output
     */
    bool pinMode(int pinIndex, PinMode mode, int freq = 500);

    /**
     * @brief Sets the value for a specified pin on the EQSP32.
     * 
     * This function allows you to set the value of a pin on the EQSP32,
     * which has 16 analog and digital I/O pins.
     * Pins 1-8 also support analog input functionality,
     * while pins 9-16 support pseudo-analog output (Push-Pull PWM).
     * The pin mode will be set using the 'pinMode' function.
     * 
     * - Digital Output (DOUT): Used for standard digital output operations.
     * - Pseudo-Analog Output (AOUT): Supports pseudo-analog output for pins 9-16. The value should range from 0 to 1000, corresponding to 0% to 100% duty cycle.
     * - Power PWM Output (POUT): Generates pull-down power PWM output. The value should range from 0 to 1000, corresponding to 0% to 100% duty cycle.
     * - Relay (RELAY): Special POUT mode, starts with set power, and after a set time drops to holding power. Needs to be set to 0 before restarting the start-hold power cycle.
     * 
     * The function returns true if the configuration is successful, and false otherwise.
     * 
     * @param pinIndex The index of the pin to set the value for (1 to 16).
     * @param value The value to set for the pin. For digital pins, a non-zero value represents HIGH, and 0 represents LOW. For analog pins, the value should range from 0 to 1000, corresponding to 0% to 100%.
     * 
     * @return true if the operation is successful, false otherwise.
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::pinValue(3, 750); // Sets the value of pin 3 to 75%
     */
    bool pinValue(int pinIndex, int value);

    /**
     * @brief Reads the value of a specified pin on the EQSP32.
     * 
     * This function allows you to read the value of a pin on the EQSP32, considering different PinModes.
     * For analog input pins (AIN), it returns the analog value in mV.
     * For temperature input pins (TIN), it calculates and returns the temperature in Celsius.
     * For other pin modes, it reads the digital state of the pin and considers the specified trigger mode (default is STATE).
     * 
     * @param pinIndex The index of the pin to read the value from (1 to 16).
     * @param trigMode The trigger mode to consider when reading digital pins. Options are STATE, ON_RISING, ON_FALLING, and ON_TOGGLE. Default is STATE.
     * 
     * @return The pin value based on the PinMode. For AIN, it returns the analog value in mV. For TIN, it returns the temperature in Celsius. For other PinModes, it returns 1 if the pin state is HIGH, and 0 if the pin state is LOW. Returns -1 if the pin index is invalid.
     * 
     * @attention The trigger mode is applicable only to digital pins. For analog and temperature pins, the trigger mode is ignored.
     * For TIN pins, the temperature calculation assumes the EQ's 5V is used as reference.
     * 
     * @note When reading a pin with a specific trigger mode (e.g., ON_RISING), the function updates the internal state. If you read the pin for ON_RISING and it is actually a falling edge, the internal state is updated. On the next read for ON_FALLING, it will return false, even if the actual edge was falling during the previous read. Take this into consideration when reading pins with trigger modes.
     * 
     * @section TrigModes Trigger Modes:
     * 
     * - STATE: Returns the current digital state of the pin (HIGH or LOW).
     * - ON_RISING: Returns true when the pin transitions from LOW to HIGH (rising edge).
     * - ON_FALLING: Returns true when the pin transitions from HIGH to LOW (falling edge).
     * - ON_TOGGLE: Returns true on each transition, alternating between HIGH and LOW.
     * 
     * @example
     * Usage example:
     * int value = EQSP32::readPin(3, ON_RISING); // Reads the digital value of pin 3, considering ON_RISING trigger mode
     */
    int readPin(int pinIndex, TrigMode trigMode = STATE);    // New method to read pin values

    /**
     * @brief Configures the PWM frequency for the Power PWM Output (POUT) mode on the EQSP32.
     * 
     * This function allows you to set the PWM frequency for the Power PWM Output (POUT) mode and all special POUT modes.
     * A change on power PWM frequency will affect all pins configured in POUT mode or any speial POUT mode.
     * The PWM frequency determines the rate at which the power is pulsed.
     * The valid frequency range is between 50 Hz and 1500 Hz.
     * The default frequency is set to 1000 Hz after EQ is initialized.
     * 
     * Affected pin modes:
     * 
     * - Power PWM Output (POUT)
     *      //Special Modes
     * - Relay (RELAY)
     * 
     * @attention All pins in POUT or special POUT mode share the same POUT frequency.
     * 
     * @param freq The PWM frequency to set for the Power PWM Output (POUT) mode. By default the frequency is set to 1000 Hz.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided frequency is outside the valid range (50 Hz to 1500 Hz).
     * 
     * @note The frequency range of the power PWM output is within audible range.
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::configPOUTFreq(500); // Sets the PWM frequency for Power PWM Output (POUT) mode to 500 Hz
     */
    bool configPOUTFreq(int freq);

        // Special mode configurations
    /**
     * @brief Configures the Switch (SWT) mode on the EQSP32.
     * 
     * This function allows you to configure the Switch (SWT) mode for a specified pin on the EQSP32.
     * In SWT mode, the pin operates as a special digital input with a debouncing timer.
     * You can set the debounce time in milliseconds using the 'debounceTime_ms' parameter.
     * 
     * @param pinIndex The index of the pin to configure for Switch (SWT) mode (1 to 16).
     * @param debounceTime_ms The debounce time in milliseconds. This parameter determines the time window during which rapid changes in the pin state are filtered to prevent false triggering. Default is 100 milliseconds.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided pin index is out of range (1 to 16).
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::configSWT(3, 150); // Configures pin 3 for Switch (SWT) mode with a debounce time of 150 milliseconds
     */
    bool configSWT(int pinIndex, int debounceTime_ms = 100);

    /**
     * @brief Configures the Temperature Input (TIN) mode on the EQSP32.
     * 
     * This function allows you to configure the Temperature Input (TIN) mode for a specified pin on the EQSP32.
     * In TIN mode, the pin operates as a special analog input mode designed for automatic temperature conversion.
     * You can set the beta coefficient and reference resistance values using the 'beta' and 'referenceResistance' parameters, respectively.
     * 
     * @param pinIndex The index of the pin to configure for Temperature Input (TIN) mode (1 to 8).
     * @param beta The beta coefficient value for temperature conversion. Default is 3988.
     * @param referenceResistance The reference resistance value for temperature conversion. Default is 10000.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided pin index is out of range (1 to 8).
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::configTIN(2, 4000, 12000); // Configures pin 2 for Temperature Input (TIN) mode with a beta coefficient of 4000 and a reference resistance of 12000 ohms
     */
    bool configTIN(int pinIndex, int beta = 3988, int referenceResistance = 10000);

    /**
     * @brief Configures the Relay (RELAY) mode on the EQSP32.
     * 
     * This function allows you to configure the Relay (RELAY) mode for a specified pin on the EQSP32.
     * In RELAY mode, the pin operates as a special Power PWM Output (POUT) mode designed for relay control.
     * You can set the hold value and derate delay using the 'holdValue' and 'derateDelay' parameters, respectively.
     * When operating in RELAY mode, the initial power is set by 'pinValue' function and when the configured derateDelay
     * has passed, the power will automatically drop to the specified holValue.
     * 
     * @param pinIndex The index of the pin to configure for Relay (RELAY) mode (1 to 16).
     * @param holdValue The hold value for relay control. This is the power level to maintain after the derate delay. Default is 500 meaning 50% of max power.
     * @param derateDelay The derate delay in milliseconds. This is the time delay before dropping to the holding power level. Default is 1000 milliseconds.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided pin index is out of range (1 to 16).
     * 
     * @example
     * Usage example:
     * bool success = EQSP32::configRELAY(4, 300, 1500); // Configures pin 4 for Relay (RELAY) mode with a hold value of 300 and a derate delay of 1500 milliseconds
     */    
    bool configRELAY(int pinIndex, int holdValue = 500, int derateDelay = 1000);

        // Read/Write database user variables
    /**
     * @brief Reads a user-defined boolean value from the EQSP32 database.
     * 
     * This function allows you to read a boolean value stored in the EQSP32 database at a specified index.
     * The function supports trigger modes for dynamic reading based on changes in the stored value.
     * The trigger modes include STATE, ON_RISING, ON_FALLING, and ON_TOGGLE (default is STATE).
     * 
     * @param idx The index of the user-defined boolean value to read (1 to USER_NUM_BOOL).
     * @param trigMode The trigger mode to consider when reading the boolean value. Options are STATE, ON_RISING, ON_FALLING, and ON_TOGGLE. Default is STATE.
     * 
     * @return The boolean value based on the specified trigger mode. Returns true or false depending on the stored value in the EQSP32 database. Returns false if the provided index is out of range (1 to USER_NUM_BOOL).
     * 
     * @note When reading a boolean value with a specific trigger mode (e.g., ON_RISING), the function updates the internal state. If you read the boolean for ON_RISING and it is actually a falling edge, the internal state is updated. On the next read for ON_FALLING, it will return false, even if the actual edge was falling during the previous read. Take this into consideration when reading boolean values with trigger modes.
     * 
     * @example
     * Usage example:
     * bool value = EQSP32::readUserBool(2, ON_TOGGLE); // Reads the user-defined boolean value at index 2 with ON_TOGGLE trigger mode
     */
    bool readUserBool(int idx, TrigMode trigMode = STATE);

    /**
     * @brief Reads a user-defined integer value from the EQSP32 database.
     * 
     * This function allows you to read an integer value stored in the EQSP32 database at a specified index.
     * 
     * @param idx The index of the user-defined integer value to read (1 to USER_NUM_INT).
     * 
     * @return The integer value stored in the EQSP32 database at the specified index. Returns 0 if the provided index is out of range (1 to USER_NUM_INT).
     * 
     * @example
     * Usage example:
     * int value = EQSP32::readUserInt(3); // Reads the user-defined integer value at index 3
     */
    int readUserInt(int idx);

    /**
     * @brief Writes a boolean value to the EQSP32 database at a user-defined index.
     * 
     * This function allows you to write a boolean value to the EQSP32 database at a specified index.
     * 
     * @param idx The index where the boolean value will be stored (1 to USER_NUM_BOOL).
     * @param value The boolean value to be stored in the EQSP32 database.
     * 
     * @example
     * Usage example:
     * EQSP32::writeUserBool(2, true); // Writes the boolean value 'true' to the user-defined index 2 in the EQSP32 database
     */
    void writeUserBool(int idx, bool value);

    /**
     * @brief Writes an integer value to the EQSP32 database at a user-defined index.
     * 
     * This function allows you to write an integer value to the EQSP32 database at a specified index.
     * 
     * @param idx The index where the integer value will be stored (1 to USER_NUM_INT).
     * @param value The integer value to be stored in the EQSP32 database.
     * 
     * @example
     * Usage example:
     * EQSP32::writeUserInt(3, 42); // Writes the integer value '42' to the user-defined index 3 in the EQSP32 database
     */
    void writeUserInt(int idx, int value);

        // DAC
    /**
     * @brief Sets the output voltage for a specified pin on the EQSP32 module using DAC functionality.
     * 
     * This function is intended for use with models of the EQSP32A models, that support analog functionality.
     * It sets the output voltage on the specified pin in millivolts.
     * The output can be set from 0 mV to 5000 mV.
     * 
     * @param pinIndex Can be EQ_AO_1 or EQ_AO_2, for analog output pin 1 and pin 2 of EQSP32A model.
     * @param mVout Desired output voltage in millivolts. The value will be clamped to the DAC's supported voltage range if it is out of bounds.
     * 
     * @return True if the analog functionality is supported and the supported pins are used, false otherwise.
     * 
     * @example
     * EQSP32 eqsp32;
     * eqsp32.begin();
     * 
     * // Set a DAC output of 2500 millivolts on pin 1
     * bool success = eqsp32.dacValue(EQ_AO_1, 2500);
     * if (success) {
     *     Serial.println("DAC value set successfully.");
     * } else {
     *     Serial.println("Operation failed or not supported.");
     * }
     */
    bool dacValue(int pinIndex, float mVout);
    
        // Buzzer
    /**
     * @brief Turns on the buzzer with a specified frequency.
     * 
     * This function activates the buzzer on the EQSP32 module at a given frequency within a specified range. 
     * The frequency can be set between 50Hz and 20KHz. If the frequency is outside this 
     * range, the buzzer will not be activated. The default frequency, if not specified, is 500 Hz.
     * 
     * @param freq The frequency in Hertz (Hz) at which to operate the buzzer. The valid range is from 50Hz 
     *        to 20KHz. The default frequency is 500 Hz.
     * 
     * @note The function will not activate the buzzer if the frequency is outside the specified range of 50Hz-20KHz.
     *      To deactivate the buzzer, function buzzerOff() should be called.
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * eqsp32.buzzerOn(); // Turns on the buzzer at the default frequency of 500 Hz
     * delay(200);
     * eqsp32.buzzerOn(1000); // Turns on the buzzer at 1000 Hz after 200ms have passed
     * delay(200);
     * eqsp32.buzzerOff();     // Turns off the buzzer after 200ms
     */
    void buzzerOn(uint32_t freq = 500);

    /**
     * @brief Turns off the buzzer on the EQSP32 module.
     * 
     * This function deactivates the buzzer by setting its duty cycle to 0%. It ensures that the buzzer is 
     * completely turned off and not emitting any sound. This is achieved by reinitializing the MCPWM 
     * (Motor Control Pulse Width Modulation) unit used for the buzzer control with a duty cycle of 0%.
     * 
     * @note This function should be called to turn off the buzzer, after it has been activated using `buzzerOn()`.
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * eqsp32.buzzerOn(1000);  // Turns on the buzzer at 1000 Hz
     * delay(200);
     * eqsp32.buzzerOff();     // Turns off the buzzer after 200ms
     */
    void buzzerOff();

        // Power sensing
    /**
     * @brief Reads the input voltage supplied to the EQSP32 module in millivolts.
     * 
     * This function retrieves the current input voltage level supplied to the EQSP32 module and returns the value in millivolts (mV).
     * Monitoring the input voltage is crucial for understanding the power supply status and ensuring the module is operating within safe voltage limits.
     * 
     * @return The input voltage level supplied to the EQSP32 module, measured in millivolts (mV).
     *
     * @example
     * EQSP32 eqsp32;
     * int inputVoltage = eqsp32.readInputVoltage();
     */
    int readInputVoltage();

    /**
     * @brief Reads the output voltage provided by the EQSP32 module in millivolts.
     * 
     * This function measures and returns the current output voltage level being provided by the EQSP32 module. 
     * The returned value is in millivolts (mV). This measurement is essential for monitoring the output power 
     * status and ensuring that the module is delivering the correct voltage to connected devices or systems.
     * The 5V output is also used as referenece voltage for TIN mode.
     * 
     * @return The output voltage level being provided by the EQSP32 module, measured in millivolts (mV).
     * 
     * @example
     * EQSP32 eqsp32;
     * int outputVoltage = eqsp32.readOutputVoltage();
     */
    int readOutputVoltage();

        // RS232/RS485 serial communication
    /**
     * @brief Configures the serial communication mode and baud rate for the EQSP32 module.
     * 
     * This function sets up the EQSP32's serial communication according to the specified mode and baud rate.
     * It supports RS232, RS485 Transmit (RS485_TX), and RS485 Receive (RS485_RX) modes. The function initializes 
     * the serial communication with the new baud rate if it differs from the previous one, and configures 
     * the transmission and reception control pins accordingly. The default mode is RS232, and the default baud rate is 115200.
     * 
     * @param mode The serial communication mode to set. Available options are RS232, RS485_TX, and RS485_RX. 
     *        The default mode is RS232.
     * @param baud The baud rate for serial communication. If the baud rate is different from the previous setting, 
     *        the function reinitializes the serial communication with the new baud rate. The default baud rate is 115200.
     * 
     * @return true if the configuration is successful and the mode is valid, false otherwise.
     * 
     * @attention If no value is provided for baud, it is assumed to be 115200. So if it is needed to use for example RS485
     *          in transmit at 9600 and then switch to receive 9600, at both times 9600 should be set in cofigSerial()
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * bool isConfigured = eqsp32.configSerial(); // Configures the module for RS232 mode at 115200 baud (default values)
     * bool isConfiguredCustom = eqsp32.configSerial(RS485_TX, 9600); // Configures the module for RS485 Transmit mode at 9600 baud
     */
    bool configSerial(EQSerialMode mode = RS232, int baud = 115200);

    bool configCAN(CanBitRates CAN_BITRATE);
    bool configCAN(CanBitRates CAN_BITRATE, uint32_t canID, bool canOpenFrame=false);

    bool transmitCANFrame(CanMessage canMessage);
    bool receiveCANFrame(CanMessage &canMessage);

    void printLocalTime();
    int getLocalHour();
    int getLocalMins();

private:
    class EQ_Private;       // Forward declaration of the nested private class
    EQ_Private* eqPrivate;
};

class Timer {
public:
    Timer(unsigned long preset = 0);

    bool start(unsigned long preset = 0);
    void stop();
    void pause();
    bool reset(unsigned long preset = 0);

    unsigned long value();
    bool isExpired();
    bool isRunning();

private:
    unsigned long startMillis;
    unsigned long presetValue;
    unsigned long elapsedTime;
};
#endif