#ifndef EQSP32_h
#define EQSP32_h

#include <Arduino.h>
#include <string>

#include "time.h"
#include <HTTPClient.h>

#include <unordered_map>   // For std::unordered_map
#include <map>             // For std::map
#include <functional>      // For std::function
#include <type_traits>     // For type traits utilities

#include "driver/ledc.h"
#include "driver/mcpwm.h"
#include "driver/twai.h"

#include <Ethernet.h>

#define POUT_PATCH(eq, pin, mode)    do {int native = eq.getPin(pin); eq.pinMode(pin, mode); pinMode(native, OUTPUT);} while (0);

extern EthernetClient eqEthernetClient;  // User-accessible Ethernet client

/**
 *      EQSP32 Main unit pin codes
 */
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

/**
 *      EQX - EQSP32 Expansion module pin codes and channels
 */
// ==========================
// EQXTC - Thermocouple Module
// ==========================
#define EQXTC_CH_1       1  // Thermocouple channel 1
#define EQXTC_CH_2       2  // Thermocouple channel 2
#define EQXTC_CH_3       3  // Thermocouple channel 3
#define EQXTC_CH_4       4  // Thermocouple channel 4
#define EQXTC_CH_5       5  // Thermocouple channel 5
#define EQXTC_CH_6       6  // Thermocouple channel 6
#define EQXTC_CHANNELS   6  // Total number of channels

// ==========================
// EQXPT - PT100 RTD Module
// ==========================
#define EQXPT_M_1        1  // PT100 Measurement point 1
#define EQXPT_M_2        2  // PT100 Measurement point 2
#define EQXPT_CHANNELS   2  // Total number of channels

// ==========================
// EQXPH - pH Measurement Module
// ==========================
#define EQXPH_PH_1       1  // pH measurement channel
#define EQXPH_CHANNELS   1  // Total number of channels

// ==========================
// EQXIO - Digital IO Expansion Module
// ==========================
#define EQXIO_PIN_1      1  // IO Pin 1
#define EQXIO_PIN_2      2  // IO Pin 2
#define EQXIO_PIN_3      3  // IO Pin 3
#define EQXIO_PIN_4      4  // IO Pin 4
#define EQXIO_PIN_5      5  // IO Pin 5
#define EQXIO_PIN_6      6  // IO Pin 6
#define EQXIO_PIN_7      7  // IO Pin 7
#define EQXIO_PIN_8      8  // IO Pin 8
#define EQXIO_PIN_9      9  // IO Pin 9
#define EQXIO_PIN_10     10 // IO Pin 10
#define EQXIO_CHANNELS   10 // Total number of channels


// EQSP32 IoT expansion modules - Masks
#define PIN_SHIFT(id)           ((id) << 0)       // Bits 0-7
                                                // Bits 8-11 (Reserved)
#define MODULE_IDX_SHIFT(id)    ((id) << 12)      // Bits 12-15
#define MODULE_SHIFT(id)        ((id) << 16)      // Bits 16-23
#define SLAVE_SHIFT(id)         ((id) << 24)      // Bits 24-31

#define PIN_MASK            PIN_SHIFT(0xFF)         // 8-bit
#define MODULE_IDX_MASK     MODULE_IDX_SHIFT(0xF)   // 4-bit        (EQX Modules)
#define MODULE_MASK         MODULE_SHIFT(0xFF)      // 8-bit        (EQX Modules)
#define SLAVE_MASK          SLAVE_SHIFT(0xFF)       // 8-bit

#define PIN_UNSHIFT(id)         (((id) & PIN_MASK) >> 0)
#define MODULE_IDX_UNSHIFT(id)  (((id) & MODULE_IDX_MASK) >> 12)    // (EQX Modules)
#define MODULE_UNSHIFT(id)      (((id) & MODULE_MASK) >> 16)        // (EQX Modules)
#define SLAVE_UNSHIFT(id)       (((id) & SLAVE_MASK) >> 24)

// EQSP32 IoT expansion modules - Index Codes
#define COMBINED_MASK       (MODULE_MASK | MODULE_IDX_MASK | PIN_MASK)
#define MASTER(pin)         (SLAVE_SHIFT(0) | (pin & MODULE_MASK) | (pin & MODULE_IDX_MASK) | (pin & PIN_MASK))
#define SLAVE_1(pin)        (SLAVE_SHIFT(1) | (pin & MODULE_MASK) | (pin & MODULE_IDX_MASK) | (pin & PIN_MASK))
// #define SLAVE_1(pin)        (SLAVE_SHIFT(1) | (pin & COMBINED_MASK))
#define SLAVE_2(pin)        (SLAVE_SHIFT(2) | (pin & MODULE_MASK) | (pin & MODULE_IDX_MASK) | (pin & PIN_MASK))
#define SLAVE_3(pin)        (SLAVE_SHIFT(3) | (pin & MODULE_MASK) | (pin & MODULE_IDX_MASK) | (pin & PIN_MASK))
#define SLAVE_4(pin)        (SLAVE_SHIFT(4) | (pin & MODULE_MASK) | (pin & MODULE_IDX_MASK) | (pin & PIN_MASK))

#define EQ_MAIN(pin)        MASTER(pin)
#define EQ_AUX_1(pin)       SLAVE_1(pin)
#define EQ_AUX_2(pin)       SLAVE_2(pin)
#define EQ_AUX_3(pin)       SLAVE_3(pin)
#define EQ_AUX_4(pin)       SLAVE_4(pin)

// (EQX Modules)
#define MAX_MODULE_TYPES    0xFF
#define EQXIO_ID            0x01        // ADIO module                  (Supported)
#define EQXSTEP_ID          0x02        // Stepper driver module
#define EQXPH_ID            0x10        // PH sensor module             (Supported)
#define EQXTC_ID            0x20        // Thermocouple sensor module   (Supported)
#define EQXPT_ID            0x30        // PT100/PT1000 sensor module   (Supported PT100)
#define EQXCI_ID            0x50        // Current input sensor [4-20mA analog input]
#define EQXCS_ID            0x50        // Current sensor module
#define EQXLC_ID            0x60        // Load cell sensor module


#define EQXIO(idx, pin)     (MODULE_SHIFT(EQXIO_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXPH(idx, pin)     (MODULE_SHIFT(EQXPH_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXTC(idx, pin)     (MODULE_SHIFT(EQXTC_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXPT(idx, pin)     (MODULE_SHIFT(EQXPT_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)

// EQSP32 pin modes
enum PinMode : uint8_t {
    NO_MODE = 0xFF,
    CUSTOM  = 0xFE,
    INIT_NA = 0xFD,
    DIN     = 0,        //                                                  |IOEXP pin LOW (1-8), HIGH (9-16)
    DOUT,               // Not used
    AIN,                //                                                  |IOEXP pin LOW (1-8)
    CIN,                // Current input mode   (Must hasIOEXP be true)     |IOEXP pin HIGH (1-8)
    AOUT,               // Not used
    POUT,               //                                                  |IOEXP pin LOW (1-8), LOW (9-16)
        // Special modes
    SWT     = 8,        // Special DIN mode with debouncing timer
    TIN,                // Special AIN mode, automatic temperature convertion
    RELAY,              // Special POUT mode, starts with set power and after set time drops to holding power
    RAIN,               // Relative analog input, this returns a value of 0-1000 representing the % of read value versus the VOut reference voltage

    PH      = 0x10,     // pH measurement       (EQX Modules)
    TC,                 // thermocouple         (EQX Modules)
    PT100_24W,          // PT100 RTD in 2/4 wire configuration      (EQX Modules)
    PT100_3W,           // PT100 RTD in 3 wire configuration        (EQX Modules)
};


#define TIN_OPEN_CIRCUIT    -9999       // Open circuit detected
#define TIN_SHORT_CIRCUIT   9999        // Short circuit detected
#define IS_TIN_VALID(value) ((value) != TIN_OPEN_CIRCUIT && (value) != TIN_SHORT_CIRCUIT)


#define TC_FAULT_OPEN       0x8001      // Thermocouple open circuit
#define TC_FAULT_SHORT_GND  0x8002      // Thermocouple shorted to GND
#define TC_FAULT_SHORT_VCC  0x8004      // Thermocouple shorted to VCC
#define IS_TC_VALID(value) (((value) & 0xFF8000) != 0x8000)        // Macro to check if a Thermocouple (TC) sensor value is valid (we check if error bit or if negative temperature)


#define PT_FAULT_THR_HIGH   0x800080    // PT sensor RTD > High allowed threshold
#define PT_FAULT_THR_LOW    0x800040    // PT sensor RTD < Low allowed threshold
#define PT_FAULT_REFIN_LOW  0x800020    // PT sensor REF under expected
#define PT_FAULT_REFIN_HIGH 0x800010    // PT sensor open circuit or REF over expected
#define PT_FAULT_RTDIN_LOW  0x800008    // PT sensor M- or I- open, or damaged RTD sensor
#define PT_FAULT_OVUV       0x800004    // PT sensor Over/Under voltage
#define IS_PT_VALID(value) (((value) & 0xFF800000) != 0x800000)      // Macro to check if a PT sensor value is valid (we check error bit or if negative temperature)


enum TrigMode {
    STATE,
    ON_RISING,
    ON_FALLING,
    ON_TOGGLE
};

enum EQ_WifiStatus : uint8_t {
    EQ_WF_DISCONNECTED  = 0,
    EQ_WF_CONNECTED,
    EQ_WF_RECONNECTING,
    EQ_WF_SCANNING
};

enum EQ_WeekDay : uint8_t {
    EQ_SUNDAY = 0,
    EQ_MONDAY,    // 1
    EQ_TUESDAY,   // 2
    EQ_WEDNESDAY, // 3
    EQ_THURSDAY,  // 4
    EQ_FRIDAY,    // 5
    EQ_SATURDAY   // 6
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
    std::string mqttBrokerIp = "homeassistant.local";
    int mqttBrokerPort = 1883;
    std::string mqtt_broker_ca = "";    // CA certificate (empty if not required)
    std::string devSystemIcon = "";     // Link to developer's system icon for IoT UI display
    std::string devSystemID = "";        // Developer system ID (assigned by the system's developer, hardcoded by developer, READ ONLY access by external app)
    std::string userDevName = "";        // Device name (assigned by the end user, default value assigned on first flash, user has READ/WRITE access)
    std::string wifiSSID = "";          // (Optional) Default network SSID
    std::string wifiPassword = "";      // (Optional) Default network password
    bool relaySequencer = false;
    bool mqttDiscovery = false;
    bool disableErqosIoT = false;
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
    HardwareSerial& Serial;  // Reference to Serial2

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
    
    // Only for self-testing, NOT to be used by the user
    void beginTest(std::string command);

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
     * uint32_t pinCode = EQXPH(EQ_PIN_3); // Example pin code for expansion module
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
     * - Temperature Input (TIN): Configures the pin as a special analog input mode for automatic temperature conversion in celsius * 10 (needs /10.0 to get actual temperature as float with 0.1 precision).
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

    /**
     * @brief Reads the value of a specified pin on the EQSP32.
     * 
     * This function allows you to read the value of a pin on the EQSP32, considering different PinModes.
     * For analog input pins (AIN), it returns the analog value in mV.
     * For temperature input pins (TIN), it calculates and returns the temperature in Celsius * 10 (need to divide by 10.0 to get actual value with 0.1 precision).
     * For other pin modes, it reads the digital state of the pin and considers the specified trigger mode (default is STATE).
     * 
     * @param pinIndex The index of the pin to read the value from (1 to 16).
     * @param trigMode The trigger mode to consider when reading digital pins. Options are STATE, ON_RISING, ON_FALLING, and ON_TOGGLE. Default is STATE.
     * 
     * @return The pin value based on the PinMode. For AIN, it returns the analog value in mV. For TIN, it returns the temperature in Celsius * 10.
     * For other input type PinModes, it returns 1 if the pin state is HIGH, and 0 if the pin state is LOW. Returns -1 if the pin index is invalid.
     * For output type PinModes, it returns the user set pinValue. Ex. if eqsp32.pinValue(2, 500), eqsp32.readPin(2) will return 500 (assuming it is set in one of the oupute modes).
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
     * The valid frequency range is between 50 Hz and 3000 Hz.
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
    bool configTIN(int pinIndex, int beta = 3435, int referenceResistance = 10000);

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
     * @brief Activates the buzzer with a specified frequency and optional duration.
     * 
     * This function turns on the buzzer on the EQSP32 module at a specified frequency within a valid range. 
     * The frequency can be set between 50 Hz and 20 KHz. If the frequency is outside this range, the 
     * buzzer will not be activated. Additionally, a duration can be specified for how long the buzzer 
     * will remain on. If no duration is specified (default is 0), the buzzer will remain on until the 
     * `buzzerOff()` function is called.
     * 
     * @param freq The frequency in Hertz (Hz) at which to operate the buzzer. The valid range is 50 Hz 
     *        to 20 KHz. The default frequency is 500 Hz.
     * @param duration_ms The duration in milliseconds for which the buzzer should remain on. If set to 0 
     *        (or not specified), the buzzer will remain on indefinitely until `buzzerOff()` is called. 
     *        The default value is 0.
     * 
     * @note If the frequency is outside the valid range of 50 Hz-20 KHz, the buzzer will not be activated.
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * eqsp32.buzzerOn(); // Activates the buzzer at the default frequency of 500 Hz, stays on indefinitely
     * delay(200);
     * eqsp32.buzzerOn(1000, 500); // Activates the buzzer at 1000 Hz for 500 ms after 200 ms delay
     * delay(600);
     * eqsp32.buzzerOff(); // Deactivates the buzzer (in this case, no effect since the buzzer turned off after 500 ms)
     */
    void buzzerOn(uint32_t freq = 500, uint32_t duration_ms = 0);

    /**
     * @brief Turns off the buzzer on the EQSP32 module.
     * 
     * This function stops the buzzer and ensures it is no longer emitting any sound. 
     * It should be used to turn off the buzzer after it has been activated with `buzzerOn()`, 
     * especially if no duration was specified or if the buzzer needs to be turned off early.
     * 
     * @example
     * Usage example:
     * EQSP32 eqsp32;
     * eqsp32.buzzerOn(1000, 0);  // Turns on the buzzer at 1000 Hz, remains on indefinitely
     * delay(200);
     * eqsp32.buzzerOff();        // Turns off the buzzer after 200 ms
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

        // CAN-Bus communication
    /**
     * @brief Configures the CAN bus interface on the EQSP32.
     * 
     * This function sets up the CAN bus peripheral using the specified bit rate,
     * with optional filtering based on the provided CAN ID and loopback operation.
     * It supports both normal and self-receiving (loopback) modes and allows you
     * to filter incoming messages to only those matching a specific 11-bit standard CAN identifier.
     * 
     * - If `canID` is set to `0` (default), the EQSP32 will accept all incoming CAN messages.
     * - If a specific `canID` is provided, only messages with that exact 11-bit ID will be received.
     * - If `loopBack` is set to `true` (default is `false`), the controller enters self-test mode
     *   and will also receive its own transmitted messages (useful for testing and development).
     * 
     * The CAN interface will be initialized at the specified bitrate using the internal TWAI controller.
     * 
     * @param CAN_BITRATE The bit rate to initialize the CAN interface with (e.g., CAN_500K).
     * @param canID The 11-bit identifier to filter incoming messages. Default is 0 (accept all).
     * @param loopBack Enable loopback mode for internal testing. Default is false (normal operation).
     * 
     * @return true if the configuration is successful, false otherwise.
     * 
     * @example
     * Usage example:
     * eqsp32.configCAN(CAN_500K);                        // Accept all messages
     * eqsp32.configCAN(CAN_125K, 0x120);                 // Accept only messages with ID 0x120
     * eqsp32.configCAN(CAN_250K, 0x000, true);           // Enable loopback (self-receive)
     */
    bool configCAN(CanBitRates CAN_BITRATE, uint32_t canID = 0, bool loopBack = false);

    /**
     * @brief Configures the CAN bus to filter messages by Node ID only (CANopen-style).
     * 
     * This setup accepts all Function Codes for the specified Node ID,
     * making it ideal for CANopen-style or other node-addressed protocols.
     * 
     * @note Broadcast messages like NMT (0x000) or SYNC (0x080) are NOT received,
     * since they use Node ID 0 and are outside this filter.
     * For accepting broadcast messages like NMT, configCAN() should be used to accept ALL IDs and implement software fitlering
     * for function codes, NMT (0x000), SYNC (0x080), etc.
     * 
     * This filter always runs in normal mode (for loopback testing use configCAN()).
     * 
     * @param CAN_BITRATE The bit rate to initialize the CAN interface with (e.g., CAN_500K).
     * @param nodeID The Node ID to filter for (1–127).
     * 
     * @return true if configuration is successful and node ID within valid range, false otherwise.
     * 
     * @example
     * eqsp32.configCANNode(CAN_500K, 0x21);    // Accepts all PDOs, SDOs, EMCY for Node ID 33 (0x21).
     */
    bool configCANNode(CanBitRates CAN_BITRATE, uint8_t nodeID);

    /**
     * @brief Transmits a CAN message using the EQSP32 CAN interface.
     * 
     * This function sends a standard 11-bit CAN message using the built-in TWAI controller.
     * The message structure must be pre-filled with an identifier, data payload, and data length.
     * Transmission is automatically handled, including support for loopback mode if enabled.
     * 
     * The function is non-blocking—if the CAN bus is not ready or the transmit queue is full,
     * it returns immediately with `false`.
     * 
     * @param canMessage A `CanMessage` struct containing the CAN ID, data bytes, and data length.
     *                   Extended and remote frames are not supported with this transmission method.
     * 
     * @return true if the message was successfully queued for transmission, false otherwise.
     * 
     * @example
     * CanMessage msg = {};
     * msg.identifier = 0x120;
     * msg.data_length_code = 3;
     * msg.data[0] = 0xAA;
     * msg.data[1] = 0xBB;
     * msg.data[2] = 0xCC;
     * 
     * bool success = eqsp32.transmitCANFrame(msg);  // Transmit the CAN message
     */
    bool transmitCANFrame(CanMessage canMessage);

    /**
     * @brief Receives a CAN message using the EQSP32 CAN interface.
     * 
     * This function attempts to retrieve a standard 11-bit CAN frame from the TWAI receive queue.
     * It operates in a non-blocking manner; if no message is available, the function returns immediately with `false`.
     * 
     * The received data, including the identifier, payload, and length, is stored in the provided `CanMessage` structure.
     * 
     * @param canMessage A reference to a `CanMessage` struct where the received data will be stored.
     * 
     * @return true if a message was successfully received and stored in the struct, false otherwise.
     * 
     * @example
     * CanMessage msg;
     * if (eqsp32.receiveCANFrame(msg)) {
     *     Serial.printf("Received CAN frame: ID=0x%03X DLC=%d\n", msg.identifier, msg.data_length_code);
     * }
     */
    bool receiveCANFrame(CanMessage &canMessage);

    // TODO add function description
    EQ_WifiStatus getWiFiStatus();

    /**
     * @brief Prints the current local time to the serial output in a human-readable format.
     * 
     * This function retrieves the current local time using the configured NTP server or default system time if the NTP is not synchronized. 
     * If the local time cannot be obtained, it logs an error message. Otherwise, it prints the time in the format:
     * "Day, Month Date Year Hour:Minute:Second".
     * 
     * @note The function uses a timeout to retrieve the local time. If synchronization fails within this time, 
     *       the function logs an error and exits.
     * 
     * @attention Ensure that the NTP server has been successfully configured and the system time is synchronized to get an accurate timestamp.
     * 
     * @example
     * EQSP32 eqsp32;
     * eqsp32.printLocalTime(); // Prints the current local time or logs an error if unavailable.
     */
    void printLocalTime();

    /**
     * @brief Checks if the local time is synchronized with the NTP server.
     * 
     * This function verifies whether the local system time has been synchronized with the NTP server
     * or if only the default system time is available.
     * 
     * @return true if the local time is synchronized with the NTP server, false only default time is available.
     * 
     * @attention This function is helpful to determine whether the timestamps used for logging and operations are accurate.
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     // Proceed with time-dependent operations
     * } else {
     *     // Handle unsynchronized time
     * }
     */
    bool isLocalTimeSynced();

    
    /**
     * @brief Retrieves the current weekday from the local time.
     * 
     * This function returns the current day of the week based on the local system time. 
     * If the local time cannot be obtained within the timeout, the function logs an error and returns EQ_SUNDAY.
     * 
     * The return value corresponds to the EQ_WeekDay enum, where:
     * - EQ_SUNDAY = 0
     * - EQ_MONDAY = 1
     * - EQ_TUESDAY = 2
     * - EQ_WEDNESDAY = 3
     * - EQ_THURSDAY = 4
     * - EQ_FRIDAY = 5
     * - EQ_SATURDAY = 6
     * 
     * @return The current local weekday as an EQ_WeekDay enum. If unsuccessful, returns EQ_SUNDAY.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * EQ_WeekDay weekday = eqsp32.getLocalWeekDay();
     * ::Serial.printf("Current Weekday (Numeric): %d\n", weekday);
     * 
     * @example
     * // Check if today is Monday
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     if (eqsp32.getLocalWeekDay() == EQ_MONDAY) {
     *         ::Serial.println("Today is Monday!");
     *     } else {
     *         ::Serial.println("It is not Monday.");
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual weekday.");
     * }
     */
    EQ_WeekDay getLocalWeekDay();

    /**
     * @brief Retrieves the current day of the year from the local time.
     * 
     * This function returns the current day of the year (1-366) based on the local system time.
     * If the local time cannot be obtained within the timeout, the function logs an error and returns 0.
     * 
     * The returned value represents the day count from the start of the year:
     * - January 1st returns 1
     * - December 31st returns 365 (or 366 in a leap year)
     * 
     * @return The current local day of the year (1-366). If unsuccessful, returns 0.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int yearDay = eqsp32.getLocalYearDay();
     *     ::Serial.printf("Today is the %dth day of the year.\n", yearDay);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual day of the year.");
     * }
     * 
     * @example
     * // Check if today is the first day of the year
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     if (eqsp32.getLocalYearDay() == 1) {
     *         ::Serial.println("Happy New Year!");
     *     } else {
     *         ::Serial.printf("We are on day %d of the year.\n", eqsp32.getLocalYearDay());
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual day of the year.");
     * }
     */
    int getLocalYearDay();


    /**
     * @brief Retrieves the current year from the local time.
     * 
     * This function returns the current year based on the local system time.
     * If the local time cannot be obtained within the timeout, the function logs an error and returns 0.
     * 
     * The returned value is the full four-digit year (e.g., 2024).
     * 
     * @return The current local year (e.g., 2024). If unsuccessful, returns 0.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int year = eqsp32.getLocalYear();
     *     ::Serial.printf("Current Year: %d\n", year);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual year.");
     * }
     * 
     * @example
     * // Check if the current year is valid and synchronized
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int year = eqsp32.getLocalYear();
     *     if (year > 2000) {
     *         ::Serial.printf("The current year is: %d\n", year);
     *     } else {
     *         ::Serial.println("Invalid year, time may not be correctly set.");
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual year.");
     * }
     */
    int getLocalYear();


    /**
     * @brief Retrieves the current month from the local time.
     * 
     * This function returns the current month (1-12) based on the local system time.
     * If the local time cannot be obtained within the timeout, the function logs an error and returns 0.
     * 
     * The returned value represents the month as:
     * - January returns 1
     * - February returns 2
     * - ...
     * - December returns 12
     * 
     * @return The current local month (1-12). If unsuccessful, returns 0.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int month = eqsp32.getLocalMonth();
     *     ::Serial.printf("Current Month: %d\n", month);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual month.");
     * }
     * 
     * @example
     * // Check if it is December
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     if (eqsp32.getLocalMonth() == 12) {
     *         ::Serial.println("It is December!");
     *     } else {
     *         ::Serial.printf("The current month is: %d\n", eqsp32.getLocalMonth());
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual month.");
     * }
     */
    int getLocalMonth();


    /**
     * @brief Retrieves the current day of the month from the local time.
     * 
     * This function returns the current day of the month (1-31) based on the local system time.
     * If the local time cannot be obtained within the timeout, the function logs an error and returns 0.
     * 
     * The returned value represents the specific day in the current month:
     * - The first day of the month returns 1
     * - The last day of the month (varies from 28 to 31) returns the respective value
     * 
     * @return The current local day of the month (1-31). If unsuccessful, returns 0.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int day = eqsp32.getLocalMonthDay();
     *     ::Serial.printf("Current Day of the Month: %d\n", day);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual day of the month.");
     * }
     * 
     * @example
     * // Check if today is the first of the month
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     if (eqsp32.getLocalMonthDay() == 1) {
     *         ::Serial.println("Today is the first day of the month!");
     *     } else {
     *         ::Serial.printf("We are on day %d of the month.\n", eqsp32.getLocalMonthDay());
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual day of the month.");
     * }
     */
    int getLocalMonthDay();


    /**
     * @brief Retrieves the current hour from the local time.
     * 
     * This function returns the current hour (0-23) based on the local system time. If the local time cannot be 
     * obtained within the timeout, the function logs an error and returns 0.
     * 
     * @return The current local hour (0-23) if successful, 0 otherwise.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * int hour = eqsp32.getLocalHour();
     * ::Serial.printf("Current Hour: %d\n", hour);
     * 
     * @example
     * // Check synchronization first
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int hour = eqsp32.getLocalHour();
     *     ::Serial.printf("Current Hour: %d\n", hour);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual local hour.");
     * }
     */
    int getLocalHour();

        /**
     * @brief Retrieves the current minutes from the local time.
     * 
     * This function returns the current minutes (0-59) based on the local system time. If the local time cannot be 
     * obtained within the timeout, the function logs an error and returns 0.
     * 
     * @return The current local minutes (0-59) if successful, 0 otherwise.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * int mins = eqsp32.getLocalMins();
     * ::Serial.printf("Current Minutes: %d\n", mins);
     * 
     * @example
     * // Check synchronization first
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int mins = eqsp32.getLocalMins();
     *     ::Serial.printf("Current Minutes: %d\n", mins);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual local minutes.");
     * }
     */
    int getLocalMins();

    /**
     * @brief Retrieves the current seconds from the local time.
     * 
     * This function returns the current seconds (0-59) based on the local system time.
     * If the local time cannot be obtained within the timeout, the function logs an error and returns 0.
     * 
     * The returned value represents the current second count within the minute:
     * - The start of a new minute returns 0
     * - The last second before the next minute returns 59
     * 
     * @return The current local seconds (0-59). If unsuccessful, returns 0.
     * 
     * @attention This function will return the default time if isLocalTimeSynced() is false!
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     int seconds = eqsp32.getLocalSecs();
     *     ::Serial.printf("Current Seconds: %d\n", seconds);
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual seconds.");
     * }
     * 
     * @example
     * // Check if we are at the start of a new minute
     * EQSP32 eqsp32;
     * if (eqsp32.isLocalTimeSynced()) {
     *     if (eqsp32.getLocalSecs() == 0) {
     *         ::Serial.println("A new minute has started!");
     *     } else {
     *         ::Serial.printf("Current second: %d\n", eqsp32.getLocalSecs());
     *     }
     * } else {
     *     ::Serial.println("Local time is not synchronized. Unable to fetch actual seconds.");
     * }
     */
    int getLocalSecs();


    long getLocalUnixTimestamp();
    std::string getFormattedLocalTime();
    long getUnixTimestamp();
    std::string getFormattedUnixTimestamp();

private:
    class EQ_Private;       // Forward declaration of the nested private class
    EQ_Private* eqPrivate;
};


/**
 * @brief EQTimer class for managing timing operations.
 * 
 * The EQTimer class provides methods to start, stop, pause, reset, and check the status of a timer.
 * It is useful for timing events and managing delays in the EQSP32 application.
 */
class EQTimer {
public:
    /**
     * @brief Constructs a EQTimer object with an optional preset value.
     * 
     * Initializes a EQTimer object, setting an optional preset value for the timer.
     * The timer is initially stopped.
     * 
     * @param preset Optional preset value in milliseconds for the timer. Default is 0.
     * 
     * @example
     * Usage example:
     * 
     * @code
     * EQTimer myTimer(5000); // Creates a EQTimer object with a 5 second preset
     * @endcode
     */
    EQTimer(unsigned long preset = 0);

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
     * EQTimer myTimer;
     * bool started = myTimer.start(3000); // Starts the timer with a 3 second preset
     * if (started) {
     *     Serial.println("EQTimer started.");
     * } else {
     *     Serial.println("EQTimer was already running.");
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
     * EQTimer myTimer;
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
     * EQTimer myTimer;
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
     * EQTimer myTimer;
     * myTimer.start(5000); // Start the timer with a 5 second preset
     * delay(2000); // Wait for 2 seconds
     * bool reset = myTimer.reset(3000); // Reset the timer with a new 3 second preset
     * if (reset) {
     *     Serial.println("EQTimer reset successfully.");
     * } else {
     *     Serial.println("EQTimer was not running.");
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
     * EQTimer myTimer;
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
     * EQTimer myTimer;
     * myTimer.start(3000); // Start the timer with a 3 second preset
     * delay(3500); // Wait for 3.5 seconds
     * if (myTimer.isExpired()) {
     *     Serial.println("EQTimer has expired.");
     * } else {
     *     Serial.println("EQTimer is still running.");
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
     * EQTimer myTimer;
     * myTimer.start(5000); // Start the timer with a 5 second preset
     * delay(6000); // Wait for 6 seconds
     * if (myTimer.isRunning()) {
     *     Serial.println("EQTimer is running.");
     * } else {
     *     Serial.println("EQTimer is not running.");
     * }
     * @endcode
     */
    bool isRunning();

private:
    unsigned long startMillis;
    unsigned long presetValue;
    unsigned long elapsedTime;
};


/*  ***********************************************
     ---   ---   ---   ---   ---   ---   ---   ---
    MQTT Device Interfacing Entities for Home Assistant (Node-RED or other MQTT platforms)
     ---   ---   ---   ---   ---   ---   ---   ---
    ***********************************************   */
/**
 * 
 *      Control and Display entities for Home Assistant integration over MQTT
 *      {These entities will read/write MQTT topics which may be used in other integration like Node-RED}
 * 
 */
// Overloads for when developing for HA only
void createControl_Switch(std::string name, std::string iconType_HA = "");
void createControl_Value(std::string name, int minValue, int maxValue, int decimals, std::string iconType_HA = "");

bool readControl_Switch(const std::string& name);
float readControl_Value(const std::string& name);

bool updateControl_Switch(const std::string& name, bool value);
bool updateControl_Value(const std::string& name, float value);

// Overloads for when developing for HA only
// binSensorType_HA (which could be omitted) could be one of the binary sensor types listed at https://www.home-assistant.io/integrations/binary_sensor/
void createDisplay_BinarySensor(std::string name, std::string iconType_HA = "", std::string binSensorType_HA = "");
// sensorType_HA (which could be omitted) could be one of the sensor types listed at https://www.home-assistant.io/integrations/sensor/
void createDisplay_Sensor(std::string name, int decimals, std::string unit, std::string iconType_HA = "", std::string sensorType_HA = "");

bool readDisplay_BinarySensor(const std::string& name);
float readDisplay_Sensor(const std::string& name);

bool updateDisplay_BinarySensor(const std::string& name, bool value);
bool updateDisplay_Sensor(const std::string& name, float value);



// Supported icons for `iconType` parameter (Beta EQ IoT app)
#define WATER_ICON          "water"
#define FIRE_ICON           "fire"
#define AIR_ICON            "air"
#define BUBBLES_ICON        "bubbles"
#define BATTERY_ICON        "battery"
#define MULTIMETER_ICON     "multimeter"
#define CALENDAR_ICON       "calendar"
#define PUMP_ICON           "pump"
#define SPRINKLER_ICON      "sprinkler"
#define FIRE_SPRINKLER_ICON "fire_sprinkler"
#define IRRIGATION_ICON     "irrigation"
#define DOOR_ICON           "door"
#define GARAGE_DOOR_ICON    "garage_door"
#define WINDOW_ICON         "window"
#define SHUTTER_ICON        "shutter"
#define SPEED_ICON          "speed"
#define DISTANCE_ICON       "distance"
#define ANGLE_ICON          "angle"
#define FORCE_ICON          "force"
#define TORQUE_ICON         "torque"
#define PRESSURE_ICON       "pressure"
#define TEMPERATURE_ICON    "temperature"
#define HUMIDITY_ICON       "humidity"
#define LIGHT_ICON          "light"
#define SOUND_ICON          "sound"
#define AIR_QUALITY_ICON    "air_quality"
#define ELECTRICAL_MEASUREMENTS_ICON    "electrical_measurements"
#define TIME_ICON           "time"
#define DURATION_ICON       "duration"
#define PH_ICON             "ph"
#define CO2_ICON            "co2"


/**
 * 
 *      Control entities (Same as configuration entities for Home Assistant) (Beta EQ IoT app)
 * 
 */
void createControl_Switch(std::string name, std::string accessLevel, std::string iconType, std::string iconType_HA = "");
void createControl_Value(std::string name, std::string accessLevel, std::string iconType, int minValue, int maxValue, int decimals, std::string unit, std::string iconType_HA = "");


/**
 * 
 *      Display entities (Beta EQ IoT app)
 * 
 */
void createDisplay_BinarySensor(std::string name, std::string accessLevel, std::string iconType, std::string onType, std::string iconType_HA = "", std::string binSensorType_HA = "");
void createDisplay_Sensor(std::string name, std::string accessLevel, std::string iconType, int decimals, std::string unit, std::string iconType_HA = "", std::string sensorType_HA = "");


/**
 * 
 *      Configuration entities (Same as control entities for Home Assistant) (Beta EQ IoT app)
 * 
 */
void createConfig_Switch(std::string name, std::string accessLevel, std::string iconType, std::string iconType_HA = "");
void createConfig_Value(std::string name, std::string accessLevel, std::string iconType, int minValue, int maxValue, int decimals, std::string unit, std::string iconType_HA = "");
// Overloads for when developing for HA only
void createConfig_Switch(std::string name, std::string iconType_HA = "");
void createConfig_Value(std::string name, int minValue, int maxValue, int decimals, std::string iconType_HA = "");

bool readConfig_Switch(const std::string& name);
float readConfig_Value(const std::string& name);

bool updateConfig_Switch(const std::string& name, bool value);
bool updateConfig_Value(const std::string& name, float value);

#endif