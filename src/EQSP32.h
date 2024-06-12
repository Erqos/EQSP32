#ifndef EQSP32_h
#define EQSP32_h

#include <Arduino.h>
#include "time.h"
#include <HTTPClient.h>

#include <unordered_map>   // For std::unordered_map
#include <map>             // For std::map
#include <functional>      // For std::function
#include <type_traits>     // For type traits utilities

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

// EQSP32 IoT expansion modules - Masks
#define PIN_SHIFT(id)       (id << 0)
#define MODULE_SHIFT(id)    (id << 16)
#define SLAVE_SHIFT(id)     (id << 24)

#define PIN_MASK            PIN_SHIFT(0xFF)
#define MODULE_MASK         MODULE_SHIFT(0xFF)
#define SLAVE_MASK          SLAVE_SHIFT(0xFF)

// EQSP32 IoT expansion modules - Index Codes
#define MASTER(pin)         (SLAVE_SHIFT(0) | (pin & MODULE_MASK) | (pin & PIN_MASK))
#define SLAVE_1(pin)        (SLAVE_SHIFT(1) | (pin & MODULE_MASK) | (pin & PIN_MASK))
#define SLAVE_2(pin)        (SLAVE_SHIFT(2) | (pin & MODULE_MASK) | (pin & PIN_MASK))
#define SLAVE_3(pin)        (SLAVE_SHIFT(3) | (pin & MODULE_MASK) | (pin & PIN_MASK))
#define SLAVE_4(pin)        (SLAVE_SHIFT(4) | (pin & MODULE_MASK) | (pin & PIN_MASK))

#define EQ_MAIN(pin)        MASTER(pin)
#define EQ_AUX_1(pin)       SLAVE_1(pin)
#define EQ_AUX_2(pin)       SLAVE_2(pin)
#define EQ_AUX_3(pin)       SLAVE_3(pin)
#define EQ_AUX_4(pin)       SLAVE_4(pin)

#define PH_EXP(pin)         (MODULE_SHIFT(1) | (pin & PIN_MASK))

// EQSP32 pin modes
enum PinMode : uint8_t {
    NO_MODE = 0xFF,
    CUSTOM  = 0xFE,
    INIT_NA = 0xFD,
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
    std::string databaseURL = "";
    std::string databaseAPIKey = "";
    std::string devSystemID = "";        // Developer system ID (assigned by the system's developer, hardcoded by developer, READ ONLY access by external app)
    std::string userDevName = "";        // Device name (assigned by the end user, default value assigned on first flash, user has READ/WRITE access)
    std::string wifiSSID = "";          // (Optional) Default network SSID
    std::string wifiPassword = "";      // (Optional) Default network password
    bool relaySequencer = false;
    bool mqttDiscovery = false;
} EQSP32Configs;


class EQ_Private;       // Forward declaration of the nested private class


/**
 * @brief EQSP32 ESP32 Industrial IoT PLC Controller.
 * 
 * The EQSP32 class provides a comprehensive interface for interacting with the EQSP32 IoT controller.
 * This class includes methods for initializing the module, configuring and controlling ADIO (analog/digital I/O) pins and their special modes, handling industrial communications (RS232, RS485, CANBus),
 * handling user cloud variables, IoT connectivity with cloud server (Firebase), and handling
 * MQTT device interfaces for Home Assistant integration.
 * 
 * @note It is critical to call the `begin()` function at the beginning of your EQSP32 application for proper operation.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * #include <EQSP32.h>
 * 
 * EQSP32 eqsp32;
 * 
 * void setup() {
 *     EQSP32Configs configs;
 *     configs.databaseURL = "https://example.com/database";    // Needed for firebase connection
 *     configs.databaseAPIKey = "API_KEY";                      // Needed for firebase connection
 *     configs.devSystemID = "SYSTEM_ID";                       // Needed for custom mobile app
 *     configs.userDevName = "Device Name";                     // Optional
 *     configs.wifiSSID = "YourSSID";                           // Optional
 *     configs.wifiPassword = "YourPassword";                   // Optional
 *     configs.relaySequencer = true;                           // Default false
 *     configs.mqttDiscovery = true;                            // Optional
 * 
 *     eqsp32.begin(configs, true);
 * }
 * 
 * void loop() {
 *     // Your main code
 * }
 * @endcode
 */
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
     * @brief Checks if a given pin identifier corresponds to a local pin on the EQSP32 module.
     * 
     * This function determines whether a given pin identifier (idMaskCode) corresponds to a local pin on the EQSP32 module.
     * A local pin refers to a pin on the EQSP32 module itself based on its mode (master, slave 1, etc).
     * 
     * For example, if the EQSP32 module is set to slave mode with ID 1, then SLAVE_1(pin) is local. If the EQSP32 module 
     * is in slave mode with ID 2 or in master mode, then SLAVE_1(pin) would return false.
     * 
     * @param idMaskCode The pin identifier mask code to check.
     * 
     * @return Returns true if the pin identifier corresponds to a local pin on the EQSP32 module, otherwise returns false.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * EQSP32 eqsp32;
     * uint32_t pinCode = SLAVE_1(EQ_PIN_3); // Example pin code
     * bool isLocal = eqsp32.isLocalPin(pinCode);
     * if (isLocal) {
     *     Serial.println("The pin is local to the EQSP32 module.");
     * } else {
     *     Serial.println("The pin is not local to the EQSP32 module.");
     * }
     * @endcode
     */
    bool isLocalPin(uint32_t idMaskCode);

    /**
     * @brief Checks if a given pin identifier corresponds to an expansion module pin on the EQSP32.
     * 
     * This function determines whether a given pin identifier (idMaskCode) corresponds to a pin on one of the expansion modules
     * of the EQSP32. It checks if the pin is not on the main unit but on an expansion module.
     * 
     * @param idMaskCode The pin identifier mask code to check.
     * 
     * @return Returns true if the pin identifier corresponds to a pin on an expansion module, otherwise returns false.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * EQSP32 eqsp32;
     * uint32_t pinCode = PH_EXP(EQ_PIN_3); // Example pin code for expansion module
     * bool isExpPin = eqsp32.isExpModulePin(pinCode);
     * if (isExpPin) {
     *     Serial.println("The pin is on an expansion module.");
     * } else {
     *     Serial.println("The pin is on the main unit.");
     * }
     * @endcode
     */
    bool isExpModulePin(uint32_t idMaskCode);

    /**
     * @brief Retrieves the corresponding ESP32 pin number for a given EQSP32 pin index.
     * 
     * This function maps a high-level EQSP32 pin index to the actual ESP32 GPIO number or peripheral pin.
     * It supports mapping for ADIO pins, RS232, RS485, and CAN bus pins. For ADIO pins, it sets the pin mode to CUSTOM.
     * For serial communication pins (RS232 and RS485), it configures the hardware appropriately to support the respective RS232 or RS485 protocol.
     * For CAN pins, it returns the native ESP32 pin mapped on the CAN driver and deinitializes the CAN peripheral. It is up to the user to initialize the CAN peripheral based on their needs.
     * 
     * @param pinIndex The index of the EQSP32 pin (e.g., EQ_RS232_TX, EQ_RS485_RX).
     * Master/Slave and Expansion module masks are handled automatically.
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
     * Master/Slave and Expansion module masks are handled automatically.
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
     * @brief Reads the current mode of a specified pin on the EQSP32.
     * 
     * This function retrieves the current mode of a specified pin on the EQSP32.
     * If the pin is valid and local, it returns the current mode of the pin, else it returns `NO_MODE`.
     * 
     * @param pinIndex The index of the pin to read the mode from (1 to 16).
     * 
     * @return The current mode of the specified pin. It returns one of the following PinMode values: 
     * `DIN`, `DOUT`, `AIN`, `AOUT`, `POUT`, `SWT`, `TIN`, `RELAY`, `RAIN`, `NO_MODE`.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * EQSP32 eqsp32;
     * eqsp32.begin();
     * PinMode mode = eqsp32.readMode(EQ_PIN_3); // Reads the mode of pin 3
     * if (mode != NO_MODE) {
     *     Serial.println("The pin mode is valid and local.");
     * } else {
     *     Serial.println("The pin mode is not valid, not configured or not local.");
     * }
     * 
     * // Example for checking the mode of a slave ID 1 pin
     * uint32_t slavePin = SLAVE_1(EQ_PIN_3);
     * PinMode slaveMode = eqsp32.readMode(slavePin); // Reads the mode of slave ID 1 pin 3
     * if (slaveMode != NO_MODE) {
     *     Serial.println("The slave pin mode is valid and local for Slave 1.");
     * } else {
     *     Serial.println("The slave pin mode is not valid, not configured or this unit is not in Slave 1 mode.");
     * }
     * @endcode
     */
    PinMode readMode(int pinIndex);

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
    // bool pinValue(IoTIndexCode iotIndex, int pinIndex, int value);

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
    // int readPin(IoTIndexCode iotIndex, int pinIndex, TrigMode trigMode = STATE);

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
    // bool configPOUTFreq(IoTIndexCode iotIndex, int freq);

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
    // bool configSWT(IoTIndexCode iotIndex, int pinIndex, int debounceTime_ms = 100);

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
    // bool configTIN(IoTIndexCode iotIndex, int pinIndex, int beta = 3988, int referenceResistance = 10000);

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
    // bool configRELAY(IoTIndexCode iotIndex, int pinIndex, int holdValue = 500, int derateDelay = 1000);

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


/**
 * @brief Timer class for managing timing operations.
 * 
 * The Timer class provides methods to start, stop, pause, reset, and check the status of a timer.
 * It is useful for timing events and managing delays in the EQSP32 application.
 */
class Timer {
public:
    /**
     * @brief Constructs a Timer object with an optional preset value.
     * 
     * Initializes a Timer object, setting an optional preset value for the timer.
     * The timer is initially stopped.
     * 
     * @param preset Optional preset value in milliseconds for the timer. Default is 0.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer(5000); // Creates a Timer object with a 5 second preset
     * @endcode
     */
    Timer(unsigned long preset = 0);

    /**
     * @brief Starts the timer with an optional preset value.
     * 
     * Starts the timer if it is not already running.
     * If a preset value is provided, it updates the timer's preset value.
     * 
     * @param preset Optional preset value in milliseconds. Default is 0.
     * @return Returns true if the timer started successfully, otherwise returns false if the timer was already running.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * bool started = myTimer.start(3000); // Starts the timer with a 3 second preset
     * if (started) {
     *     Serial.println("Timer started.");
     * } else {
     *     Serial.println("Timer was already running.");
     * }
     * @endcode
     */
    bool start(unsigned long preset = 0);

    /**
     * @brief Stops the timer.
     * 
     * Stops the timer and resets the elapsed time.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(5000); // Start the timer
     * delay(2000); // Wait for 2 seconds
     * myTimer.stop(); // Stop the timer
     * @endcode
     */ 
    void stop();

    /**
     * @brief Pauses the timer.
     * 
     * Pauses the timer, retaining the elapsed time. The timer can be resumed by calling `start()`.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(5000); // Start the timer
     * delay(2000); // Wait for 2 seconds
     * myTimer.pause(); // Pause the timer
     * delay(1000); // Wait for another second
     * myTimer.start(); // Resume the timer
     * @endcode
     */
    void pause();

    /**
     * @brief Resets the timer with an optional preset value.
     * 
     * Resets the timer and optionally updates the preset value. The timer stops and can be restarted.
     * 
     * @param preset Optional preset value in milliseconds. Default is 0.
     * @return Returns true if the timer was running and has been reset, otherwise returns false.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(5000); // Start the timer with a 5 second preset
     * delay(2000); // Wait for 2 seconds
     * bool reset = myTimer.reset(3000); // Reset the timer with a new 3 second preset
     * if (reset) {
     *     Serial.println("Timer reset successfully.");
     * } else {
     *     Serial.println("Timer was not running.");
     * }
     * @endcode
     */
    bool reset(unsigned long preset = 0);

    /**
     * @brief Retrieves the current elapsed time of the timer.
     * 
     * Returns the total elapsed time since the timer was started, including any paused periods. 
     * When the timer is paused, the elapsed time stops accumulating until the timer is resumed.
     * 
     * @return The elapsed time in milliseconds.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(10000); // Start the timer with a 10 second preset
     * delay(3000); // Run for 3 seconds
     * myTimer.pause(); // Pause the timer
     * delay(2000); // Pause for 2 seconds (does not count towards the elapsed time)
     * myTimer.start(); // Resume the timer
     * delay(3000); // Run for another 3 seconds
     * unsigned long elapsedTime = myTimer.value(); // Retrieve the elapsed time
     * Serial.print("Elapsed time: ");
     * Serial.println(elapsedTime); // Should print 6000 (milliseconds)
     * @endcode
     */
    unsigned long value();

    /**
     * @brief Checks if the timer has expired.
     * 
     * Determines if the timer has reached or exceeded its preset value without stopping or resetting the timer.
     * The timer will continue running even after it has expired.
     * 
     * @return Returns true if the timer has reached or exceeded its preset value, otherwise returns false.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(3000); // Start the timer with a 3 second preset
     * delay(3500); // Wait for 3.5 seconds
     * if (myTimer.isExpired()) {
     *     Serial.println("Timer has expired.");
     * } else {
     *     Serial.println("Timer is still running.");
     * }
     * @endcode
     */
    bool isExpired();

    /**
     * @brief Checks if the timer is currently running.
     * 
     * Determines if the timer is active and counting time. The timer will continue running even after reaching
     * its preset value, and this method will return true as long as the timer has not been stopped or paused.
     * 
     * @return Returns true if the timer is running, otherwise returns false.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * Timer myTimer;
     * myTimer.start(5000); // Start the timer with a 5 second preset
     * delay(6000); // Wait for 6 seconds
     * if (myTimer.isRunning()) {
     *     Serial.println("Timer is running.");
     * } else {
     *     Serial.println("Timer is not running.");
     * }
     * @endcode
     */
    bool isRunning();

private:
    unsigned long startMillis;
    unsigned long presetValue;
    unsigned long elapsedTime;
};


/*  **********************************************
    =============================================
            MQTT Device Interfacing Entities
    =============================================
    **********************************************   */
/*      Define MQTT interface type strings (when not creating a custom sensor or binary sensor)      */
// Control Interfaces
#define SWITCH                    "switch"
#define BUTTON                    "button"
#define VALVE                     "valve"

#define INTEGER_VALUE             "integer"
#define FLOAT_VALUE               "float"

// Sensor Interfaces
#define WATER_CONSUMPTION_SENSOR  "water"
#define VOLUME_STORAGE_SENSOR     "volume_storage"
#define VOLUME_FLOW_RATE_SENSOR   "volume_flow_rate"
#define PRECIPITATION_INTENSITY_SENSOR "precipitation_intensity"

struct BinarySensorEntity {
  std::string devClass              = "";
  std::string icon                  = "";   // Optional
};

struct SensorEntity {
  std::string devClass              = "";
  std::string unit_of_measurement   = "";
  int display_precision             = 0;
  std::string icon                  = "";   // Optional
};

/*  *****************
    Create Interface
    *****************   */
/**
 * @brief Exposes a specified pin as an MQTT entity.
 * 
 * This function exposes a specified pin on the EQSP32 as an MQTT entity, allowing it to be monitored and controlled through Home Assistant. 
 * The entity is created based on the pin's mode, and its properties are set accordingly.
 * 
 * @param name The name assigned to the MQTT entity.
 * @param pin The pin number to be exposed as an MQTT entity.
 * 
 * @note This function will only create the entity if the pin is local for the respective EQSP32 mode (master, slave 1, etc).
 * 
 * @example
 * Usage example:
 * 
 * @code
 * createInterface("Temperature Sensor", EQ_PIN_3); // Exposes pin 3 as a temperature sensor entity if this module is in master mode
 * @endcode
 */
void createInterface(std::string name, int pin);

/**
 * @brief Creates a custom MQTT entity for Home Assistant based on a predefined type.
 * Check "MQTT interface type strings" definitions for the implemented types.
 * 
 * This function creates a custom MQTT entity for Home Assistant using a predefined type. The EQSP32 implementation automatically decides 
 * the specific entity type (e.g., sensor, switch) based on the type name provided.
 * 
 * @param name The name assigned to the MQTT entity.
 * @param type The predefined type of the MQTT entity. Valid types are: "switch", "button", "valve", "integer", "float", 
 *   "water", "volume_storage", "volume_flow_rate", "precipitation_intensity".
 * 
 * @example
 * Usage example:
 * 
 * @code
 * createInterface("Water Consumption", "water"); // Creates a custom water consumption sensor entity
 * @endcode
 */
void createInterface(std::string name, std::string type);

/**
 * @brief Creates a binary sensor MQTT entity for Home Assistant.
 * 
 * This function creates a binary sensor MQTT entity for Home Assistant, which users can read and update for automation purposes. 
 * The properties of the binary sensor are specified by the user.
 * 
 * @param name The name assigned to the MQTT entity.
 * @param sensor The properties of the binary sensor entity, including:
 * - `devClass`: The device class of the binary sensor (e.g., "battery", "battery_charging", "carbon_monoxide", "cold", "connectivity", 
 *   "door", "garage_door", "gas", "heat", "light", "lock", "moisture", "motion", "moving", "occupancy", "opening", "plug", "power", 
 *   "presence", "problem", "running", "safety", "smoke", "sound", "tamper", "update", "vibration", "window").
 * - `icon` (Optional): The icon to use for the binary sensor entity.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * BinarySensorEntity doorSensor;
 * doorSensor.devClass = "door";
 * doorSensor.icon = "mdi:door";    // Optional, if omitted the default devClass icon will be used automatically
 * createInterface("Front Door", doorSensor); // Creates a binary sensor entity for the front door
 * @endcode
 */
void createInterface(std::string name, BinarySensorEntity sensor);

/**
 * @brief Creates a sensor MQTT entity for Home Assistant.
 * 
 * This function creates a sensor MQTT entity for Home Assistant, which users can read and update for automation purposes. 
 * The properties of the sensor are specified by the user.
 * 
 * @param name The name assigned to the MQTT entity.
 * @param sensor The properties of the sensor entity, including:
 * - `devClass`: The device class of the sensor (e.g., "apparent_power", "aqi", "atmospheric_pressure", "battery", 
 *   "carbon_dioxide", "carbon_monoxide", "current", "data_rate", "data_size", "date", "distance", "duration", "energy", 
 *   "energy_storage", "enum", "frequency", "gas", "humidity", "illuminance", "irradiance", "moisture", "monetary", 
 *   "nitrogen_dioxide", "nitrogen_monoxide", "nitrous_oxide", "ozone", "ph", "pm1", "pm25", "pm10", "power_factor", 
 *   "power", "precipitation", "precipitation_intensity", "pressure", "reactive_power", "signal_strength", "sound_pressure", 
 *   "speed", "sulphur_dioxide", "temperature", "timestamp", "volatile_organic_compounds", 
 *   "volatile_organic_compounds_parts", "voltage", "volume", "volume_flow_rate", "volume_storage", "water", "weight", 
 *   "wind_speed").
 * - `unit_of_measurement`: The unit of measurement for the sensor's value (e.g., "°C", "%").
 * - `display_precision`: The number of decimal places to display for the sensor's value.
 * - `icon` (Optional): The icon to use for the sensor entity.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * SensorEntity temperatureSensor;
 * temperatureSensor.devClass = "temperature";
 * temperatureSensor.unit_of_measurement = "°C";
 * temperatureSensor.display_precision = 1;
 * temperatureSensor.icon = "mdi:thermometer";
 * createInterface("Room Temperature", temperatureSensor); // Creates a temperature sensor entity for the room
 * @endcode
 */
void createInterface(std::string name, SensorEntity sensor);

/*  *****************
    Read Interface
    *****************   */
/**
 * @brief Reads the value of a specified interface.
 * 
 * This template function reads the value of a specified interface by its name. It can return values of different types
 * (e.g., bool, int, float, std::string) based on the type specified during the function call. If the interface is not found,
 * or the value is empty or not in the value map, it returns a default value based on the type.
 * 
 * @tparam T The type of the value to be read (e.g., bool, int, float, std::string).
 * @param interfaceName The name of the interface to read the value from.
 * @return The value of the interface, or a default value if the interface is not found or the value is empty.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * bool boolValue = readInterface<bool>("MyBoolInterface"); // Reads a boolean value from the interface
 * int intValue = readInterface<int>("MyIntInterface"); // Reads an integer value from the interface
 * float floatValue = readInterface<float>("MyFloatInterface"); // Reads a float value from the interface
 * std::string stringValue = readInterface<std::string>("MyStringInterface"); // Reads a string value from the interface
 * @endcode
 */
template <typename T> T readInterface(const std::string& interfaceName);        // Template function to read the interface

// Specific functions
/**
 * @brief Reads a boolean value from a specified interface.
 * 
 * This function reads a boolean value from a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to read the boolean value from.
 * @return The boolean value of the interface, or false if the interface is not found or the value is empty.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * bool boolValue = readInterfaceBOOL("MyBoolInterface"); // Reads a boolean value from the interface
 * @endcode
 */
bool readInterfaceBOOL(const std::string& interfaceName);

/**
 * @brief Reads an integer value from a specified interface.
 * 
 * This function reads an integer value from a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to read the integer value from.
 * @return The integer value of the interface, or 0 if the interface is not found or the value is empty.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * int intValue = readInterfaceINT("MyIntInterface"); // Reads an integer value from the interface
 * @endcode
 */
int readInterfaceINT(const std::string& interfaceName);

/**
 * @brief Reads a float value from a specified interface.
 * 
 * This function reads a float value from a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to read the float value from.
 * @return The float value of the interface, or 0.0f if the interface is not found or the value is empty.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * float floatValue = readInterfaceFLOAT("MyFloatInterface"); // Reads a float value from the interface
 * @endcode
 */
float readInterfaceFLOAT(const std::string& interfaceName);

/**
 * @brief Reads a string value from a specified interface.
 * 
 * This function reads a string value from a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to read the string value from.
 * @return The string value of the interface, or an empty string if the interface is not found or the value is empty.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * String stringValue = readInterfaceSTRING("MyStringInterface"); // Reads a string value from the interface
 * @endcode
 */
String readInterfaceSTRING(const std::string& interfaceName);

/*  *****************
    Update Interface
    *****************   */
/**
 * @brief Updates the state of a specified interface.
 * 
 * This template function updates the state of a specified interface by its name.
 * If the interface is not found, an appropriate message is logged.
 * 
 * @tparam T The type of the value to be updated (e.g., bool, int, float, std::string).
 * @param interfaceName The name of the interface to update the state for.
 * @param value The new state value to update the interface with.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * updateInterface("MyBoolInterface", true); // Updates the boolean value of the interface
 * updateInterface("MyIntInterface", 42); // Updates the integer value of the interface
 * updateInterface("MyFloatInterface", 3.14f); // Updates the float value of the interface
 * updateInterface("MyStringInterface", "New Value"); // Updates the string value of the interface
 * @endcode
 */
template <typename T> void updateInterface(const std::string& interfaceName, T value);       // Function to write to the interface

/**
 * @brief Updates the boolean value of a specified interface.
 * 
 * This function updates the boolean value of a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to update the boolean value for.
 * @param value The new boolean value to update the interface with.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * updateInterfaceBOOL("MyBoolInterface", true); // Updates the boolean value of the interface
 * @endcode
 */
void updateInterfaceBOOL(const std::string& interfaceName, bool value);

/**
 * @brief Updates the integer value of a specified interface.
 * 
 * This function updates the integer value of a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to update the integer value for.
 * @param value The new integer value to update the interface with.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * updateInterfaceINT("MyIntInterface", 42); // Updates the integer value of the interface
 * @endcode
 */
void updateInterfaceINT(const std::string& interfaceName, int value);

/**
 * @brief Updates the float value of a specified interface.
 * 
 * This function updates the float value of a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to update the float value for.
 * @param value The new float value to update the interface with.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * updateInterfaceFLOAT("MyFloatInterface", 3.14f); // Updates the float value of the interface
 * @endcode
 */
void updateInterfaceFLOAT(const std::string& interfaceName, float value);

/**
 * @brief Updates the string value of a specified interface.
 * 
 * This function updates the string value of a specified interface by its name.
 * 
 * @param interfaceName The name of the interface to update the string value for.
 * @param value The new string value to update the interface with.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * updateInterfaceSTRING("MyStringInterface", "New Value"); // Updates the string value of the interface
 * @endcode
 */
void updateInterfaceSTRING(const std::string& interfaceName, std::string value);

/*  *****************
    Write Interface
    *****************   */
/**
 * @brief Writes a command to a specified interface and publishes the new state to MQTT.
 * 
 * This template function writes a command to a specified interface by its name. The new state value
 * is published to the MQTT topic associated with the interface. If the interface is not found, an appropriate 
 * message is logged.
 * 
 * @tparam T The type of the value to be written (e.g., bool, int, float, std::string).
 * @param interfaceName The name of the interface to write the command to.
 * @param value The new command value to write to the interface.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * writeInterface("MyBoolInterface", true); // Writes a boolean command to the interface
 * writeInterface("MyIntInterface", 42); // Writes an integer command to the interface
 * writeInterface("MyFloatInterface", 3.14f); // Writes a float command to the interface
 * writeInterface("MyStringInterface", "New Value"); // Writes a string command to the interface
 * @endcode
 */
template <typename T> void writeInterface(const std::string& interfaceName, T value);       // Function to write to the interface

/**
 * @brief Writes a boolean command to a specified interface and publishes the new state to MQTT.
 * 
 * This function writes a boolean command to a specified interface by its name and publishes the new state to the associated MQTT topic.
 * 
 * @param interfaceName The name of the interface to write the boolean command to.
 * @param value The new boolean command value to write to the interface.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * writeInterfaceBOOL("MyBoolInterface", true); // Writes a boolean command to the interface
 * @endcode
 */
void writeInterfaceBOOL(const std::string& interfaceName, bool value);

/**
 * @brief Writes an integer command to a specified interface and publishes the new state to MQTT.
 * 
 * This function writes an integer command to a specified interface by its name and publishes the new state to the associated MQTT topic.
 * 
 * @param interfaceName The name of the interface to write the integer command to.
 * @param value The new integer command value to write to the interface.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * writeInterfaceINT("MyIntInterface", 42); // Writes an integer command to the interface
 * @endcode
 */
void writeInterfaceINT(const std::string& interfaceName, int value);

/**
 * @brief Writes a float command to a specified interface and publishes the new state to MQTT.
 * 
 * This function writes a float command to a specified interface by its name and publishes the new state to the associated MQTT topic.
 * 
 * @param interfaceName The name of the interface to write the float command to.
 * @param value The new float command value to write to the interface.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * writeInterfaceFLOAT("MyFloatInterface", 3.14f); // Writes a float command to the interface
 * @endcode
 */
void writeInterfaceFLOAT(const std::string& interfaceName, float value);

/**
 * @brief Writes a string command to a specified interface and publishes the new state to MQTT.
 * 
 * This function writes a string command to a specified interface by its name and publishes the new state to the associated MQTT topic.
 * 
 * @param interfaceName The name of the interface to write the string command to.
 * @param value The new string command value to write to the interface.
 * 
 * @example
 * Usage example:
 * 
 * @code
 * writeInterfaceSTRING("MyStringInterface", "New Value"); // Writes a string command to the interface
 * @endcode
 */
void writeInterfaceSTRING(const std::string& interfaceName, std::string value);

#endif