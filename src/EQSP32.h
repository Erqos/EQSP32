#ifndef EQSP32_h
#define EQSP32_h



// ============================================================================
//                  ---         Quick API summary           ---
// ============================================================================


/** ---------------------------------------------------------------------------
 *                          ---   EQSP32 Class   ---
 *  ---------------------------------------------------------------------------
 * - EQSP32()                     Create the main EQSP32 controller object.
 * 
 *  Startup
 *  ---------------------------------------------------------------------------
 * - begin(configs, verbose)      Start EQSP32 core, hardware, modules, and background services.
 * - begin(verbose)               Start EQSP32 with default settings.
 * - beginHil()                   Enable HIL test mode before begin().
 *
 *  Pin and channel I/O
 *  ---------------------------------------------------------------------------
 * - getPin(pinIndex)             Get the native ESP32 GPIO/peripheral pin for custom use.
 * - pinMode(pinIndex, mode)      Set the mode of a main-unit pin or EQX channel.
 * - readMode(pinIndex)           Read the current mode of a main-unit pin or EQX channel.
 * - pinValue(pinIndex, value)    Set an output value for a configured output pin or channel.
 * - readPin(pinIndex, trigMode)  Read a digital, analog, sensor, counter, output, or status value.
 * 
 *  Special mode configuration
 *  ---------------------------------------------------------------------------
 * - configPOUTFreq(freq)         Set the shared POUT/RELAY PWM frequency.
 * - configPOUT(freq)             Alias for configPOUTFreq().
 * - configSWT(pin, debounce)     Set debounce timing for SWT input mode.
 * - configTIN(pin, beta, refR)   Set TIN conversion using Beta parameters.
 * - configTIN(pin, A, B, C)      Set TIN conversion using Steinhart-Hart coefficients.
 * - configTIN(pin, nA, nB, nC, refR) Set TIN conversion using normalized Steinhart-Hart coefficients.
 * - configRELAY(pin, hold, delay) Set relay hold power and derate timing.
 * - configPCC(pin, trigMode)     Set pulse-counting edge behavior.
 * - configWDG(pin, delay)        Configure watchdog behavior for supported channels.
 * 
 *  Industrial communication
 *  ---------------------------------------------------------------------------
 * - configSerial(mode, baud)     Configure RS232/RS485 serial mode and baud rate.
 * - configCAN(bitRate, id, loop) Configure the CAN interface and optional filter/loopback.
 * - configCANNode(bitRate, node) Configure CAN filtering by CANopen-style node ID.
 * - transmitCANFrame(message)    Send a CAN frame.
 * - receiveCANFrame(message)     Read a CAN frame when available.
 * 
 *  Device and network information
 *  ---------------------------------------------------------------------------
 * - localIP()                    Read the active device IP address.
 * - ethernetIP()                 Read the Ethernet IP address.
 * - wifiIP()                     Read the Wi-Fi IP address.
 * - isDeviceOnline()             Check whether the device is online through Wi-Fi or Ethernet.
 * - getDeviceName()              Read the current device name.
 * - setDeviceName(name)          Change the device name.
 * - getDeviceID()                Read the device ID.
 * 
 *  Connection status
 *  ---------------------------------------------------------------------------
 * - getWiFiStatus()              Read Wi-Fi connection status.
 * - getEthernetStatus()          Read Ethernet connection status.
 * - getMQTTStatus()              Read MQTT broker connection status.
 * - getBleStatus()               Read BLE connection/advertising status.
 * 
 *  Module and hardware status
 *  ---------------------------------------------------------------------------
 * - isModuleDetected(moduleCode) Check whether a specific EQX module is detected.
 * - isLoRaAvailable()            Check whether the onboard LoRa module is available.
 * 
 *  Supply monitoring
 *  ---------------------------------------------------------------------------
 * - readInputVoltage()           Read the EQSP32 input supply voltage in mV.
 * - readOutputVoltage()          Read the EQSP32 output/load supply voltage in mV.
 * 
 *  User button, buzzer, and LEDs
 *  ---------------------------------------------------------------------------
 * - readUserButton()             Read the onboard user/BOOT button state.
 * - buzzerOn(freq, duration)     Turn the buzzer on, optionally for a fixed time.
 * - buzzerOff()                  Turn the buzzer off.
 * - setBleLed()                  Manually turn on the BLE LED when user-controlled.
 * - resetBleLed()                Manually turn off the BLE LED when user-controlled.
 * - toggleBleLed()               Manually toggle the BLE LED when user-controlled.
 * - setWifiLed()                 Manually turn on the Wi-Fi LED when user-controlled.
 * - resetWifiLed()               Manually turn off the Wi-Fi LED when user-controlled.
 * - toggleWifiLed()              Manually toggle the Wi-Fi LED when user-controlled.
 * 
 *  Connectivity control
 *  ---------------------------------------------------------------------------
 * - startWiFi(ssid, password)    Start Wi-Fi using saved or provided credentials.
 * - stopWiFi()                   Stop Wi-Fi.
 * - clearWiFi()                  Clear saved Wi-Fi credentials.
 * - startBle()                   Start BLE advertising.
 * - stopBle()                    Stop BLE advertising.
 * - updateMQTT(user, pass, en)   Update MQTT credentials and enable or disable MQTT.
 * - updateMQTT(enable)           Enable or disable MQTT using stored credentials.
 * 
 *  Time and uptime
 *  ---------------------------------------------------------------------------
 * - printLocalTime()             Print the current local time.
 * - isLocalTimeSynced()          Check whether local time is synchronized.
 * - getLocalWeekDay()            Read local weekday.
 * - getLocalYearDay()            Read day of year.
 * - getLocalYear()               Read local year.
 * - getLocalMonth()              Read local month.
 * - getLocalMonthDay()           Read local day of month.
 * - getLocalHour()               Read local hour.
 * - getLocalMins()               Read local minute.
 * - getLocalSecs()               Read local second.
 * - getUptimeDays()              Read uptime days.
 * - getUptimeHours()             Read uptime hours.
 * - getUptimeMinutes()           Read uptime minutes.
 * - getUptimeSeconds()           Read uptime seconds.
 * - printUptime()                Print current uptime.
 * - getLocalUnixTimestamp()      Read local Unix timestamp.
 * - getFormattedLocalTime()      Read formatted local time string.
 * - getUnixTimestamp()           Read UTC Unix timestamp.
 * - getFormattedUnixTimestamp()  Read formatted UTC time string.
 * 
 *  Extra
 *  ---------------------------------------------------------------------------
 * - initStatus()                 Check whether initialization is not started, running, OK, or in error.
 * - isExpModulePin(idMaskCode)   Check whether a pin code belongs to an EQX expansion module.
 */


/** ---------------------------------------------------------------------------
 *                          ---   EQTimer Class   ---
 *  ---------------------------------------------------------------------------
 * - EQTimer(preset)              Create a stopped timer with an optional preset.
 * - start(preset)                Start or resume the timer; optional non-zero preset updates the target.
 * - stop()                       Stop the timer and clear elapsed time.
 * - pause()                      Pause the timer while preserving elapsed time.
 * - reset(preset)                Restart a running timer from zero; optional non-zero preset updates the target.
 * - value()                      Return elapsed time in milliseconds.
 * - isElapsed(autoReset)         True only when a running timer has reached its preset; can auto-reset.
 * - isExpiredDisabled()          True when the timer has elapsed, is stopped/paused, or has no preset.
 * - isExpired()                  Deprecated alias for isExpiredDisabled().
 * - isRunning()                  True while the timer is actively counting.
 */


/** ---------------------------------------------------------------------------
 *                          ---   MQTT Entities   ---
 *  ---------------------------------------------------------------------------
 *
 *  Control entities
 *  ---------------------------------------------------------------------------
 * - createControl_Switches(items, count) Create multiple switch control entities. (Recommended)
 * - createControl_Values(items, count)   Create multiple numeric control entities. (Recommended)
 * - createControl_Texts(items, count)    Create multiple text control entities. (Recommended)
 * - createControl_Switch(name, icon)     Create one switch control entity.
 * - createControl_Value(name, min, max, decimals, icon) Create one numeric control entity.
 *
 * - readControl_Switch(name)             Read a switch control value.
 * - readControl_Value(name)              Read a numeric control value.
 * - readControl_Text(name)               Read a text control value.
 *
 * - updateControl_Switch(name, value, retain) Publish a switch control state.
 * - updateControl_Value(name, value, retain)  Publish a numeric control state.
 * - updateControl_Text(name, value, retain)   Publish a text control state.
 *
 *  Display / monitoring entities
 *  ---------------------------------------------------------------------------
 * - createDisplay_BinarySensors(items, count) Create multiple binary-sensor entities. (Recommended)
 * - createDisplay_Sensors(items, count)       Create multiple sensor entities. (Recommended)
 * - createDisplay_BinarySensor(name, icon, deviceClass) Create one binary-sensor entity.
 * - createDisplay_Sensor(name, decimals, unit, icon, deviceClass) Create one sensor entity.
 *
 * - readDisplay_BinarySensor(name)       Read a binary-sensor display value.
 * - readDisplay_Sensor(name)             Read a sensor display value.
 *
 * - updateDisplay_BinarySensor(name, value, retain) Publish a binary-sensor state.
 * - updateDisplay_Sensor(name, value, retain)       Publish a sensor state.
 *
 *  Plain global topics
 *  ---------------------------------------------------------------------------
 * - createGlobal_Topic(topic)            Register a plain MQTT topic.
 * - readGlobal_Topic(topicName)          Read a plain MQTT topic as String.
 * - readGlobal_TopicBOOL(topicName)      Read a plain MQTT topic as bool.
 * - readGlobal_TopicFLOAT(topicName)     Read a plain MQTT topic as float.
 * - updateGlobal_Topic(topicName, payload, retain) Publish a plain MQTT topic payload.
 */



#include <Arduino.h>
#include <RadioLib.h>
#include <string>

#include "time.h"

#include <unordered_map>   // For std::unordered_map
#include <map>             // For std::map
#include <functional>      // For std::function
#include <type_traits>     // For type traits utilities

#include "driver/ledc.h"
#include "driver/mcpwm.h"
#include "driver/twai.h"


#ifndef EQSP32_REMOVE_NOTES         // To disable EQSP32 notes during build use {#define EQSP32_REMOVE_NOTES} before {include "EQSP32.h"}
#warning "EQXIO NOTICE: New EQXIO firmware available. If you are using the EQXIO module, please update to latest firmware version, else the EQSP32 will reject it."
#endif


#define TINY_GSM_MODEM_SIM800       // Defined the modem type used for EQX2G (SIM800)

#define EQ_USER_SPI     FSPI        // Do not use HSPI, as it is reserved for internal use


// --- EQX2G - UART Bridge
extern Stream& eqx2gStream;         // Use this directly to send serial messages to the EQX2G module, or attach it to a GSM library, like TinyGsm in <TinyGsmClient.h>
#define eqx2gSerial   eqx2gStream   // Use eqx2gStream or eqx2gSerial, it is the same


// --- BLE NUS - Nordic UART Service (reference) -----------------------------
// Service: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
// RX (Central->EQSP32, Write/WriteNR): 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
// TX (EQSP32->Central, Notify):       6E400003-B5A3-F393-E0A9-E50E24DCCA9E
// MTU: default 23 (not changed) => max payload 20 bytes per write/notify.
// ---------------------------------------------------------------------------
extern Stream& bleStream;           // It allows for Serial like communication over BLE (Nordic UART Service {NUS})
#define bleSerial   bleStream       // Use bleStream or bleSerial, it is the same


// ============================================================================
//  EQSP32 LoRa Class
//  Use eqsp32LoRa like a normal RadioLib SX1262 radio object.
// ============================================================================
class EQ_LoRaClass : public SX1262 {
public:
  EQ_LoRaClass(Module* mod);
  int16_t begin(float freq = 868.0, float bw = 125.0, uint8_t sf = 7, uint8_t cr = 5, uint8_t syncWord = RADIOLIB_SX126X_SYNC_WORD_PRIVATE, int8_t power = 10, uint16_t preambleLength = 8);
  int16_t reset(bool validate = true);
};
extern EQ_LoRaClass& eqsp32LoRa;


/**
 *      EQSP32 Main unit pin codes
 */
// User available pin codes
#define EQ_PIN_1        1
#define EQ_PIN_2        2
#define EQ_PIN_3        3
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

#define EQ_RS485_2_TX   27
#define EQ_RS485_2_RX   28
#define EQ_RS485_2_EN   29

#define IS_ADIO_PIN(p)      (p >= EQ_PIN_1 && p <= EQ_PIN_16 ? true : false)
#define IS_RS232_PIN(p)     (p == EQ_RS232_TX || p == EQ_RS232_RX ? true : false)
#define IS_RS485_PIN(p)     (p >= EQ_RS485_TX && p <= EQ_RS485_EN ? true : false)
#define IS_CAN_PIN(p)       (p == EQ_CAN_TX || p == EQ_CAN_RX ? true : false)
#define IS_2xRS485_PIN(p)   (p >= EQ_RS485_2_TX && p <= EQ_RS485_2_EN ? true : false)

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

// ==========================
// EQXAI - Analog Input Expansion Module
// ==========================
#define EQXAI_PIN_1      1  // Analog Input Pin 1
#define EQXAI_PIN_2      2  // Analog Input  Pin 2
#define EQXAI_PIN_3      3  // Analog Input  Pin 3
#define EQXAI_PIN_4      4  // Analog Input  Pin 4
#define EQXAI_PIN_5      5  // Analog Input  Pin 5
#define EQXAI_PIN_6      6  // Analog Input  Pin 6
#define EQXAI_PIN_7      7  // Analog Input  Pin 7
#define EQXAI_PIN_8      8  // Analog Input  Pin 8
#define EQXAI_CHANNELS   8  // Total number of channels

// ==========================
// EQXRA - Relay and Analog Output Expansion Module
// ==========================
//
// Relay terminal grouping:
// RL1 and RL2 have independent OUT/NO + COM.
// RL3-RL8 are paired with shared COM: 3-4, 5-6, 7-8.
    //  Physical channels - Terminals
#define EQXRA_RL_1        1   // Relay 1, independent COM
#define EQXRA_RL_2        2   // Relay 2, independent COM
#define EQXRA_RL_3        3   // Relay 3, shared COM 3-4
#define EQXRA_RL_4        4   // Relay 4, shared COM 3-4
#define EQXRA_RL_5        5   // Relay 5, shared COM 5-6
#define EQXRA_RL_6        6   // Relay 6, shared COM 5-6
#define EQXRA_RL_7        7   // Relay 7, shared COM 7-8
#define EQXRA_RL_8        8   // Relay 8, shared COM 7-8
#define EQXRA_AO_1        9   // Analog output 1 target
#define EQXRA_AO_2        10  // Analog output 2 target
    //  Internal channels - Feedback
#define EQXRA_AOFB_1      11  // Analog output 1 feedback
#define EQXRA_AOFB_2      12  // Analog output 2 feedback
#define EQXRA_RL_SUPPLY   13  // Relay coil supply voltage feedback
#define EQXRA_SYSSTAT     14  // Module system status
#define EQXRA_AOSTAT      15  // Analog output status

#define EQXRA_CHANNELS    15  // Total user-facing logical channels


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


// ==============================================================
//              EQX Module IDs      (EQX Modules)
// ==============================================================
//
// Module ID ranges:
// 0x01-0x0F : General I/O and mixed-signal modules
// 0x10-0x3F : Input sensor modules
// 0xA0-0xAF : Custom/combined modules
// 0xE0-0xEF : Communication / IoT modules

// I/O modules
#define EQXM_MAX_MODULE_TYPES   0xFF
#define EQXIO_ID                0x01        // DIO module                       (Supported)
#define EQXAI_ID                0x02        // Analog input voltage and 4-20mA  (Supported)
// #define EQXAO_ID             0x03        // Analog output voltage and 4-20mA

// Input sensor modules
// ===== Chemical / liquid sensing modules | 0x10 - 0x1F =====
#define EQXPH_ID            0x10        // PH sensor module             (Supported)
// ===== Thermocouple / high-temperature sensing modules | 0x20 - 0x2F =====
#define EQXTC_ID            0x20        // Thermocouple sensor module   (Supported)
// ===== RTD / resistance temperature sensing modules | 0x30 - 0x3F =====
#define EQXPT_ID            0x30        // PT100 sensor module          (Supported)

// Custom modules
// ===== Combined / application-specific / local-control modules | 0xA0 - 0xBF =====
#define EQXRA_ID            0xA0        // Relay + analog output combo module   (Supported)

// IoT modules
// #define EQXLORA_ID          0xE0      // Lora/LoraWan module
#define EQX2G_ID            0xE1      // 2G GSM/GPRS module             (Supported)
// ==============================================================


#define EQXIO_MACRO(idx, pin)     (MODULE_SHIFT(EQXIO_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXAI_MACRO(idx, pin)     (MODULE_SHIFT(EQXAI_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)

#define EQXPH_MACRO(idx, pin)     (MODULE_SHIFT(EQXPH_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXTC_MACRO(idx, pin)     (MODULE_SHIFT(EQXTC_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)
#define EQXPT_MACRO(idx, pin)     (MODULE_SHIFT(EQXPT_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)

#define EQXRA_MACRO(idx, pin)     (MODULE_SHIFT(EQXRA_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)

#define EQX2G_MACRO(idx, pin)     (MODULE_SHIFT(EQX2G_ID) | (MODULE_IDX_SHIFT(idx & 0x0F)) | (pin & PIN_MASK))        // (EQX Modules)

// Inline wrappers with default pin = 0
inline uint32_t EQXIO(int idx, int pin = 0) { return EQXIO_MACRO(idx, pin); }
inline uint32_t EQXAI(int idx, int pin = 0) { return EQXAI_MACRO(idx, pin); }

inline uint32_t EQXPH(int idx, int pin = 0) { return EQXPH_MACRO(idx, pin); }
inline uint32_t EQXTC(int idx, int pin = 0) { return EQXTC_MACRO(idx, pin); }
inline uint32_t EQXPT(int idx, int pin = 0) { return EQXPT_MACRO(idx, pin); }

inline uint32_t EQXRA(int idx, int pin = 0) { return EQXRA_MACRO(idx, pin); }

inline uint32_t EQX2G(int idx = 1, int pin = 0) { return EQX2G_MACRO(idx, pin); }

// EQSP32 pin modes
enum EQ_PinMode : uint8_t {
    NO_MODE = 0xFF,
    CUSTOM  = 0xFE,
    INIT_NA = 0xFD,
    DIN     = 0,        //
    DOUT,               // N/A (Do NOT use)
    AIN,                //
    CIN,                // Current input mode, returns mA x 100
    PCC,                // Pulse Capture Counter mode, counted pulses are cleared on each readPin() call for the respective pin
    POUT,               //
    AOUT,               // EQXRA (EQX Modules)  | 0 mV - 10000 mV (value range)

        // Special modes
    SWT     = 8,        // Special DIN mode with debouncing timer
    TIN,                // Special AIN mode, automatic temperature conversion
    RELAY,              // Special POUT mode, starts with set power and after set time drops to holding power
    RAIN,               // Relative analog input, this returns a value of 0-1000 representing the % of read value versus the VOut reference voltage
    SAOUT,              // EQXRA (EQX Modules)  | 0 mV - 10000 mV (value range) | Sync AOUT, special AOUT mode; Sync with the local AOUT grouped channels, ex. in EQXRA if AO1 is in SAOUT, then AO2 is also in SAOUT

    PH      = 0x10,     // pH measurement       (EQX Modules)
    TC,                 // thermocouple         (EQX Modules)
    PT100_24W,          // PT100 RTD in 2/4 wire configuration      (EQX Modules)
    PT100_3W,           // PT100 RTD in 3 wire configuration        (EQX Modules)
};


/** =========================================
 * 
 *      Value conversions based on pin modes
 *    
 * Divide the respective readPin() value 
 * with the respective multiplier for conversion.
 * 
 *   =========================================*/
#define AIN_TO_V_MULT       1000.0      // AIN values are by default in mV (V * 1000)
#define TIN_TO_C_MULT       10.0        // TIN values are in C * 10
#define CIN_TO_mA_MULT      100.0       // CIN values are in mA * 100

#define PH_TO_PH_MULT       100.0       // PH values are in pH * 100
#define TC_TO_C_MULT        10.0        // TC values are in C * 10
#define PT_TO_C_MULT         100.0       // PT values are in C * 100 (higher precision temperature measurements that the other modes)

inline float CONVERT_AIN(int readPinValue) { return ( (float)readPinValue / AIN_TO_V_MULT ); }
inline float CONVERT_TIN(int readPinValue) { return ( (float)readPinValue / TIN_TO_C_MULT ); }
inline float CONVERT_CIN(int readPinValue) { return ( (float)readPinValue / CIN_TO_mA_MULT ); }
inline float CONVERT_PH(int readPinValue) { return ( (float)readPinValue / PH_TO_PH_MULT ); }
inline float CONVERT_TC(int readPinValue) { return ( (float)readPinValue / TC_TO_C_MULT ); }
inline float CONVERT_PT(int readPinValue) { return ( (float)readPinValue / PT_TO_C_MULT ); }

// ====================================


// ============================================================================
//              EQSP32 / EQX error and status definitions
// ============================================================================

// --------------------------------------------------------------------
//              EQXAI / CIN errors
// --------------------------------------------------------------------
#define CIN_OC_ERROR        -1          // CIN sensor has detected an over current condition (> 21mA)

// --------------------------------------------------------------------
//              TIN errors
// --------------------------------------------------------------------
#define TIN_OPEN_CIRCUIT    -9999       // Open circuit detected
#define TIN_SHORT_CIRCUIT   9999        // Short circuit detected
#define IS_TIN_VALID(value) ((value) != TIN_OPEN_CIRCUIT && (value) != TIN_SHORT_CIRCUIT)

// --------------------------------------------------------------------
//              TC errors (EQXTC)
// --------------------------------------------------------------------
#define TC_FAULT_OPEN       0x8001      // Thermocouple open circuit
#define TC_FAULT_SHORT_GND  0x8002      // Thermocouple shorted to GND
#define TC_FAULT_SHORT_VCC  0x8004      // Thermocouple shorted to VCC
#define IS_TC_VALID(value) (((value) & 0xFF8000) != 0x8000)        // Macro to check if a Thermocouple (TC) sensor value is valid (we check if error bit or if negative temperature)

// --------------------------------------------------------------------
//              PT errors (EQXPT)
// --------------------------------------------------------------------
#define PT_FAULT_THR_HIGH   0x800080    // PT sensor RTD > High allowed threshold   (Wrong sensor value)
#define PT_FAULT_THR_LOW    0x800040    // PT sensor RTD < Low allowed threshold    (Wrong sensor value)
#define PT_FAULT_REFIN_LOW  0x800020    // PT sensor REF under expected             (No sensor)
#define PT_FAULT_REFIN_HIGH 0x800010    // PT sensor open circuit or REF over expected  (Open circuit)
#define PT_FAULT_RTDIN_LOW  0x800008    // PT sensor M- or I- open, or damaged RTD sensor   (Open or damaged circuit)
#define PT_FAULT_OVUV       0x800004    // PT sensor Over/Under voltage             (Voltage error)
#define IS_PT_VALID(value) (((value) & 0xFF800000) != 0x800000)      // Macro to check if a PT sensor value is valid (we check error bit or if negative temperature)

// --------------------------------------------------------------------
//             Relay and analog output module errors (EQXRA)
// --------------------------------------------------------------------
//              System status bits
// General module-level status. Specific bits report the source of the fault.
// GENERAL_FAULT is a summary bit set when any fault bit is active.
#define EQXRA_SYS_STATUS_OK                 0x00
#define EQXRA_SYS_STATUS_SW_WDG_TRIP        0x01
#define EQXRA_SYS_STATUS_RL_WDG_TRIP        0x02
#define EQXRA_SYS_STATUS_AO_WDG_TRIP        0x04
#define EQXRA_SYS_STATUS_SUPPLY_UV          0x08
#define EQXRA_SYS_STATUS_SUPPLY_OV          0x10
#define EQXRA_SYS_STATUS_DAC_ERR            0x20
#define EQXRA_SYS_STATUS_AOUT_FAULT         0x40
#define EQXRA_SYS_STATUS_GENERAL_FAULT      0x80

//              AOUT status bits
// --------------------------------------------------------------------
// One byte [AOUT2 status << 4 | AOUT1 status]:
// bits 0..3 = AOUT1 status
// bits 4..7 = AOUT2 status

// DEV = abs(target - feedback) > threshold for longer than delay
// OV  = feedback > overvoltage threshold
// SC  = target is meaningfully above zero, but feedback remains near zero

#define EQXRA_AOUT_STATUS_OK                0x00
#define EQXRA_AOUT_STATUS_CH_DEV            0x01
// #define RSRV1                            0x02        // RESERVED
#define EQXRA_AOUT_STATUS_CH_OV             0x04
#define EQXRA_AOUT_STATUS_CH_SC             0x08
#define EQXRA_AOUT_STATUS_CH_NA             0x0F

#define EQXRA_AOUT_STATUS_AO1_SHIFT         0
#define EQXRA_AOUT_STATUS_AO2_SHIFT         4

#define EQXRA_AOUT_STATUS_AO1_MASK          0x0F
#define EQXRA_AOUT_STATUS_AO2_MASK          0xF0
#define EQXRA_AOUT_STATUS_NA                0xFF        // All flags set indicating DAC Not Available

#define EQXRA_AOUT_STATUS_AO1(status)            (((status) & EQXRA_AOUT_STATUS_AO1_MASK) >> EQXRA_AOUT_STATUS_AO1_SHIFT)
#define EQXRA_AOUT_STATUS_AO2(status)            (((status) & EQXRA_AOUT_STATUS_AO2_MASK) >> EQXRA_AOUT_STATUS_AO2_SHIFT)
#define EQXRA_SAOUT_APPLY(aout)                  ((aout & 0x3FFF) | 0x8000)     // Set the apply bit for Sync AOUT mode
// ============================================================================

enum EQ_InitStatus : uint8_t {
    INIT_NOT_STARTED,    // `begin()` has not been called yet.
    INIT_STARTED,        // `begin()` has been called and initialization is still in progress.
    INIT_OK,        // Initialization completed successfully; the EQSP32 core is ready.
    INIT_ERROR      // Initialization finished but one or more errors occurred during startup.
};

enum EQ_TrigMode : uint8_t {
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

enum EQ_EthernetStatus : uint8_t {
    EQ_ETH_DISCONNECTED  = 0,   // Ethernet cable disconnected
    EQ_ETH_CONNECTED,           // Online
    EQ_ETH_PLUGGED_IN,          // Plugged in, not yet online
    EQ_ETH_STOPPED
};

enum EQ_MQTTBrokerStatus : uint8_t {
    EQ_MQTT_DISCONNECTED  = 0,   // No connection with broker, not trying
    EQ_MQTT_CONNECTED,           // Connected on MQTT broker
    EQ_MQTT_CONNECTING,          // No connection with broker, system is trying to connect
    EQ_MQTT_NO_INIT = 0xFF       // MQTT has not been setup
};

enum EQ_BleStatus : uint8_t {
    EQ_BLE_DISCONNECTED = 0,   // BLE initialized, NOT connected, and NOT advertising
    EQ_BLE_CONNECTED,          // BLE client connected, but NO active notification subscription
    EQ_BLE_ADVERTISING,        // BLE advertising and discoverable
    EQ_BLE_SUBSCRIBED,         // BLE client connected and subscribed to notifications
    EQ_BLE_NO_INIT = 0xFF      // BLE NOT initialized or NOT managed by EQSP32 library
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
    std::string mqttBrokerIp = "homeassistant.local";
    int mqttBrokerPort = 1883;
    std::string mqtt_broker_ca = "";    // CA certificate (empty if not required)
    std::string devSystemID = "";        // Developer system ID (assigned by the system's developer, hardcoded by developer, READ ONLY access by external app)
    std::string userDevName = "";        // Device name (assigned by the end user, default value assigned on first flash, user has READ/WRITE access)
    std::string wifiSSID = "";          // (Optional) Default network SSID
    std::string wifiPassword = "";      // (Optional) Default network password
    std::string staticIP = "0.0.0.0";   // (Default) (Optional) Leave as 0.0.0.0 to enable DHCP
    std::string gateway = "0.0.0.0";
    std::string subnet = "0.0.0.0";
    std::string DNS = "0.0.0.0";
    bool relaySequencer = false;
    bool mqttDiscovery = false;
    bool disableErqosIoT = false;
    bool disableNetSwitching = false;
    bool disableSystemMQTTEntities = false; // (Optional) Set to true to disable the automatic publication and update of system entities (In/Out supply voltages, heartbeat, uptime, signal strength); any of these may be implemented in the developer's code
    bool mqttAutoUpdateStateFromCommand = true; // When enabled, for every received MQTT command topic the system will automatically update/publish the received value to the corresponding state topic. When disabled, the state topic of the entity is only updated by calling the corresponding update function ex. updateControl_Switch().
    int bleBroadcastingMins = 3;        // Only if disableErqosIoT == false. BLE will broadcast for these many mins before turning off (setting to 0 makes BLE always available). If BLE turns off, pressing the BOOT button or webserver access is needed to reenable it.
} EQSP32Configs;


class EQ_Private;       // Forward declaration of the nested private class


#if ESP_ARDUINO_VERSION_MAJOR > 2
#undef Serial  // 🛡️ protect the header itself
#endif

/**
 * @brief EQSP32 ESP32-based Industrial IoT PLC Controller.
 *
 * The EQSP32 class is the main interface for configuring, controlling, and monitoring
 * the EQSP32 Industrial IoT controller.
 *
 * It provides APIs for:
 * - System initialization and lifecycle management (see `begin()`)
 * - Configuring and controlling onboard ADIO pins (digital/analog and special modes)
 * - Interfacing with EQX expansion modules (e.g., EQXIO, EQXAI, EQXPH, EQXTC, EQXPT, EQX2G)
 * - Industrial communication interfaces (RS232, RS485, CAN bus)
 * - Device/network status accessors (Wi-Fi/Ethernet state, IP address, device name/ID)
 * - Integrated MQTT device interfacing for platforms such as Home Assistant / Node-RED
 *   (MQTT topic handling and discovery are managed internally by the library)
 *
 * The EQSP32 library can manage Wi-Fi and BLE provisioning (via EQConnect) and runs
 * background system tasks required for connectivity and device services. In typical
 * applications, user code should not initialize Wi-Fi manually unless the internal IoT
 * core is explicitly disabled using `EQSP32Configs::disableErqosIoT`.
 *
 * @note
 * It is mandatory to call `begin()` before using any other EQSP32 functionality.
 *
 * @example
 * @code
 * #include <EQSP32.h>
 *
 * EQSP32 eqsp32;
 *
 * void setup() {
 *     EQSP32Configs configs;
 *
 *     // MQTT broker configuration (defaults shown)
 *     configs.mqttBrokerIp   = "homeassistant.local";
 *     configs.mqttBrokerPort = 1883;
 *     configs.mqtt_broker_ca = "";              // Optional CA certificate (empty if not required)
 *
 *     // Device identity (optional)
 *     configs.devSystemID = "SYSTEM_ID";        // Developer/system identifier
 *     configs.userDevName = "Device Name";      // User-visible device name
 *
 *     // Optional Wi-Fi defaults (can also be provisioned via EQConnect)
 *     configs.wifiSSID     = "YourSSID";
 *     configs.wifiPassword = "YourPassword";
 *
 *     // Optional static IP (leave "0.0.0.0" to use DHCP)
 *     configs.staticIP = "0.0.0.0";
 *     configs.gateway  = "0.0.0.0";
 *     configs.subnet   = "0.0.0.0";
 *     configs.DNS      = "0.0.0.0";
 *
 *     // Features / behavior
 *     configs.relaySequencer      = true;       // Default false
 *     configs.mqttDiscovery       = true;       // Default false
 *     configs.disableErqosIoT     = false;      // Default false (library manages Wi-Fi/BLE)
 *     configs.disableNetSwitching = false;      // Default false
 *
 *     // BLE advertising window (only used when disableErqosIoT == false)
 *     configs.bleBroadcastingMins = 3;          // 0 = always advertise, default 3 minutes
 *
 *     eqsp32.begin(configs, true);              // true enables verbose debug logs
 * }
 *
 * void loop() {
 *     // User application logic
 *     delay(10);
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
     * @brief Initializes the EQSP32 core and starts required background services.
     *
     * This function MUST be called once at the beginning of your application before using
     * any other EQSP32 APIs.
     *
     * What `begin()` does (high level):
     * - Initializes internal drivers and peripherals used by the library (I/O, comms, etc.).
     * - Places onboard ADIO pins into a safe default state (Digital Input) until the user configures them.
     * - Initializes serial interfaces used by EQSP32 (e.g., Serial2 reference in the class, and default comm setup).
     * - Starts the EQSP32 internal system/background tasks required for runtime services and module handling.
     *
     * Networking / IoT core behavior:
     * - If `EQSP32Configs::disableErqosIoT == false` (default), the library may manage connectivity services
     *   such as Wi-Fi/BLE provisioning (EQConnect workflow), network switching, and MQTT interfacing
     *   (including discovery when enabled).
     * - If `disableErqosIoT == true`, those internal IoT services are disabled, and the user application
     *   is responsible for any external networking behavior it needs.
     *
     * @param eq_configs Optional startup configuration (MQTT broker, device identity, Wi-Fi defaults,
     *                   static IP settings, feature flags such as relay sequencer, discovery, etc.).
     *                   If omitted, defaults from the struct are used.
     * @param verboseEnabled Enable verbose debug logs printed to Serial.
     *
     * @note
     * - Create only one `EQSP32` instance in your program and reuse it. Multiple instances can conflict
     *   due to shared hardware resources and background tasks.
     * - Call `initStatus()` if you have other tasks that might run concurrently and need to gate logic
     *   until initialization has finished.
     *
     * @example
     * @code
     * #include <EQSP32.h>
     *
     * EQSP32 eqsp32;
     *
     * void setup() {
     *   EQSP32Configs cfg;
     *   cfg.devSystemID = "SYSTEM_ID";
     *   cfg.userDevName = "Device Name";
     *   cfg.mqttBrokerIp = "homeassistant.local";
     *   cfg.mqttBrokerPort = 1883;
     *   cfg.mqttDiscovery = true;
     *   cfg.relaySequencer = true;
     *
     *   // Optional Wi-Fi defaults (can also be provisioned via EQConnect)
     *   cfg.wifiSSID = "YourSSID";
     *   cfg.wifiPassword = "YourPassword";
     *
     *   eqsp32.begin(cfg, true);   // true enables verbose logs
     * }
     *
     * void loop() {
     *   // Your application logic
     * }
     * @endcode
     */
    void begin(EQSP32Configs eq_configs = {}, bool verboseEnabled = false);

    /**
     * @brief Initializes the EQSP32 core (convenience overload).
     *
     * Equivalent to calling `begin(EQSP32Configs{}, verboseEnabled)`.
     *
     * @param verboseEnabled Enable verbose debug logs printed to Serial.
     */
    void begin(bool verboseEnabled);

    /**
     * @brief Enables Hardware-in-the-Loop (HIL) test mode.
     *
     * HIL allows an external tester to run the real user sketch without real sensors
     * or expansion-module inputs. The sketch remains unchanged: normal APIs such as
     * readPin(), pinValue(), getLocalHour(), getLocalMins(), etc. are still used.
     *
     * Call this before begin().
     *
     * In HIL mode, test data is sent over USB Serial using text frames terminated by
     * newline:
     *
     * Commands:
     * -!EQ.1=1                         Set main EQ input pin 1
     * -!XIO.1.3=1                      Set EQXIO module 1, pin 3 input state
     * -!XAI.1.2=2450                   Set EQXAI module 1, channel 2 returned value
     * -!XTC.1.1=235                    Set EQXTC module 1, channel 1 returned value
     * -!XPT.1.1=2415                   Set EQXPT module 1, channel 1 returned value
     * -!XPH.1.1=700                    Set EQXPH module 1 pH returned value
     * -!TIME=2026-06-13T08:00:00       Set ESP32 system time
     * -!RST                            Restart the device
     *
     * Queries:
     * -?EQ.1                           Read main EQ pin 1
     * -?XAI.1.2                        Read EQXAI module 1, channel 2
     * -?TIME                           Read current system time
     * -?STATUS                         Read active HIL command/query buffer status
     *
     * Multiple items may be separated with '/':
     * -!EQ.1=1/XIO.1.3=0/XAI.1.2=2450
     * -?EQ.1/XAI.1.2/TIME
     *
     * Periodic frames use #period_ms:
     * -?#500/EQ.1/XAI.1.2              Query every 500 ms
     * -!#1000/EQ.1=1                   Apply command every 1000 ms
     *
     * Send "-!" to clear the command buffer and "-?" to clear the query buffer.
     *
     * HIL overrides input/sensor values only. Output reads still return the normal
     * EQSP32 output cache set by pinValue(). EQXRA and EQX2G are not input-injected.
     *
     * @return true if HIL mode was enabled, false if EQSP32 is not ready to enter HIL.
     */
    bool beginHil();

    /**
     * @brief Retrieves the current initialization status of the EQSP32 core.
     *
     * This function reports the state of the internal EQSP32 startup sequence
     * that begins when `begin()` is called.
     *
     * Before `begin()` is invoked, this function always returns `INIT_NOT_STARTED`.
     * While `begin()` is actively executing, the state becomes `INIT_STARTED`,
     * which is only observable if queried from another task running concurrently.
     * Once `begin()` exits, the state transitions to either `INIT_OK` or
     * `INIT_ERROR`.
     *
     * In typical applications, all user logic and tasks are started after
     * `eqsp32.begin()` completes. In these cases, only `INIT_NOT_STARTED`
     * (before `begin()`) and `INIT_OK` (after `begin()`) will normally be seen.
     * The `INIT_STARTED` state exists primarily for advanced use cases where
     * other tasks need to monitor EQSP32 initialization asynchronously.
     *
     * Returned values:
     * - `INIT_NOT_STARTED`: `begin()` has not been called yet.
     * - `INIT_STARTED`: Initialization is currently in progress.
     * - `INIT_OK`: Initialization completed successfully; the EQSP32 core is ready.
     * - `INIT_ERROR`: Initialization finished but one or more errors occurred.
     *
     * @return The current EQSP32 initialization status as an `EQ_InitStatus` value.
     *
     * @note
     * - This function is non-blocking and may be called repeatedly.
     * - It is recommended to start user logic and additional tasks only after
     *   `eqsp32.begin()` has completed.
     */
    EQ_InitStatus initStatus();


    // Only for self-testing, NOT to be used by the user
    void beginTest(std::string command);


    /**
     * @brief DEPRECATED
     */
    bool isLocalPin(uint32_t idMaskCode);


    /**
     * @brief Checks if a given pin identifier corresponds to an expansion module pin/channel.
     *
     * This function determines whether the provided encoded pin/channel identifier refers to an EQX expansion module rather than the EQSP32 main unit.
     *
     * @param idMaskCode The encoded pin/channel identifier to check.
     *
     * @return `true` if the identifier refers to an EQX expansion-module pin/channel; otherwise `false`.
     *
     * @example
     * @code
     * uint32_t pinCode = EQXIO(1, EQXIO_PIN_3);  // EQXIO module 1, pin 3
     *
     * if (eqsp32.isExpModulePin(pinCode)) {
     *     Serial.println("The pin/channel is on an expansion module.");
     * } else {
     *     Serial.println("The pin/channel is on the EQSP32 main unit.");
     * }
     * @endcode
     */
    bool isExpModulePin(uint32_t idMaskCode);


    /**
     * @brief Checks whether a specific EQX expansion module is detected on the system.
     * 
     * This function allows you to check if a particular EQSP32 expansion module (EQX) is currently connected and recognized
     * by the EQSP32 controller. It verifies the presence of the module based on the provided `moduleCode`, which should
     * be constructed using the appropriate macros like `EQXTC(idx)`, `EQXPH(idx)`, `EQXIO(idx, pin)`, etc.
     * 
     * @param moduleCode The encoded module identifier. Use macros like `EQXPH(idx)`, `EQXTC(idx)`, `EQXIO(idx, pin)` to generate this value.
     *                  The `idx` argument must be in the range 1–15. Index 0 is not valid and should not be used.
     * 
     * @return true if the module is detected and properly initialized, false otherwise.
     * 
     * @note
     * - The EQSP32 system supports up to 15 expansion modules of each type.
     * - Indexing is 1-based, meaning the first module uses index 1.
     * - Indexing is per module type, not based on physical daisy-chain order.
     *   For example, if you have 2 EQXPH modules and 2 EQXIO modules connected in any order,
     *   the first detected pH module will be `EQXPH(1)` and the second will be `EQXPH(2)`, regardless of where the EQXIO modules are placed.
     *   Likewise, the first detected IO module will be `EQXIO(1)` and the second `EQXIO(2)`, even if they appear later in the chain physically.
     * 
     * @example
     * @code
     * if (eqsp32.isModuleDetected(EQXPH(1))) {
     *     Serial.println("pH module 1 detected.");
     * } else {
     *     Serial.println("ph module 1 NOT detected.");
     * }
     * @endcode
     */
    bool isModuleDetected(uint32_t moduleCode);


    /**
     * @brief Checks whether the onboard SX1262 LoRa module is available and ready for use.
     *
     * This function reports the result of the internal LoRa initialization process
     * performed during `eqsp32.begin()`. It returns `true` only if an SX1262-based
     * LoRa module was successfully detected, its variant was identified, and the
     * module was initialized correctly.
     *
     * If this function returns `false`, LoRa functionality is not available and
     * calls to `eqsp32LoRa.begin()` or other LoRa operations should be avoided.
     *
     * @return `true` if the SX1262 LoRa module is available and ready for use;
     *         `false` otherwise.
     *
     * @note
     * - This function reflects the LoRa initialization status established during
     *   `eqsp32.begin()`.
     * - Supported SX1262 variants are automatically detected by the library.
     * - A return value of `false` may indicate that no supported LoRa module is
     *   installed, the module could not be identified, or initialization failed.
     *
     * @example
     * @code
     * eqsp32.begin();
     *
     * if (eqsp32.isLoRaAvailable()) {
     *     int16_t state = eqsp32LoRa.begin();
     *
     *     if (state == RADIOLIB_ERR_NONE) {
     *         Serial.println("LoRa ready.");
     *     }
     * } else {
     *     Serial.println("LoRa module not available.");
     * }
     * @endcode
     */
    bool isLoRaAvailable();


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
     * @code
     * int rs232Tx = eqsp32.getPin(EQ_RS232_TX); // Gets the ESP32 pin number for RS232 transmission.
     * int rs232Rx = eqsp32.getPin(EQ_RS232_RX); // Gets the ESP32 pin number for RS232 transmission.
     * @endcode
     */
    int getPin(int pinIndex);


        // EQ pin configurations
    /**
     * @brief Sets the mode for an EQSP32 main-unit pin or supported EQX expansion-module channel.
     *
     * This function configures a pin/channel for one of the available `EQ_PinMode` modes.
     * Supported modes depend on the selected pin/channel, hardware version, and installed expansion module.
     *
     * Supported mode categories:
     *
     * General I/O modes:
     * - `DIN`: Digital input. Used by EQSP32 main pins and supported EQX digital-input channels.
     * - `AIN`: Analog voltage input. On the EQSP32 main unit this is available on pins 1–8; also used by supported analog-input expansion modules.
     * - `CIN`: Current input. Used by supported current-input hardware or EQX analog-input modules.
     * - `POUT`: Power PWM output. Used by EQSP32 main pins and supported EQX output channels.
     * - `PCC`: Pulse Capture Count input. On the EQSP32 main unit this is available on pins 9–16, with up to four active PCC channels.
     *
     * Special I/O modes:
     * - `SWT`: Debounced digital input. Call `configSWT()` after `pinMode()`.
     * - `TIN`: NTC temperature input. On the EQSP32 main unit this is available on pins 1–8. Call `configTIN()` after `pinMode()`. `readPin()` returns Celsius × 10.
     * - `RELAY`: Relay-optimized POUT mode. Call `configRELAY()` after `pinMode()`.
     * - `RAIN`: Relative analog input. On the EQSP32 main unit this is available on pins 1–8 and returns 0–1000 relative to 5V VOut.
     *
     * EQX module-specific modes:
     * - `AOUT`: Analog voltage output, used by supported analog-output expansion modules such as EQXRA.
     * - `SAOUT`: Synchronized analog voltage output, used by supported analog-output expansion modules such as EQXRA.
     * - `PH`: pH measurement mode, used by EQXPH.
     * - `TC`: Thermocouple temperature mode, used by EQXTC.
     * - `PT100_24W`: PT100 RTD mode for 2-wire / 4-wire configurations, used by EQXPT.
     * - `PT100_3W`: PT100 RTD mode for 3-wire configuration, used by EQXPT.
     *
     * @param pinIndex The EQSP32 pin index or encoded EQX channel code. Master/Slave and expansion-module masks are handled automatically.
     * @param mode The mode to set for the pin/channel. Unsupported mode/pin combinations return `false`.
     * @param freq Deprecated and ignored. Use `configPOUTFreq()` to set the shared POUT/RELAY PWM frequency.
     *
     * @return `true` if the mode was applied successfully; `false` if the pin/channel is invalid, unsupported, unavailable, or the requested mode is not allowed.
     *
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_3, AIN);              // Main-unit analog input
     * eqsp32.pinMode(EQ_PIN_12, PCC);             // Main-unit pulse counter input
     * eqsp32.pinMode(EQXAI(1, 1), CIN);           // EQXAI channel 1 current input, if supported
     * eqsp32.pinMode(EQXRA(1, EQXRA_AO_1), AOUT); // EQXRA analog output
     * @endcode
     */
    bool pinMode(int pinIndex, EQ_PinMode mode, int freq = 500);


    /**
     * @brief Reads the current mode of an EQSP32 main-unit pin or supported EQX expansion-module channel.
     *
     * This function returns the currently configured `EQ_PinMode` for the selected pin/channel.
     *
     * @param pinIndex The EQSP32 pin index or encoded EQX channel code to check.
     *
     * @return The current `EQ_PinMode`, such as `DIN`, `AIN`, `CIN`, `PCC`, `POUT`, `AOUT`, `SAOUT`, `SWT`, `TIN`, `RELAY`, `RAIN`, `PH`, `TC`, `PT100_24W`, `PT100_3W`,
     * or `NO_MODE` if the pin/channel is invalid, unavailable, unsupported, or not configured.
     *
     * @example
     * @code
     * EQ_PinMode mode = eqsp32.readMode(EQ_PIN_3);
     *
     * if (mode == AIN) {
     *     Serial.println("Pin is configured as analog input.");
     * }
     *
     * EQ_PinMode eqxMode = eqsp32.readMode(EQXAI(1, 1));
     *
     * if (eqxMode == CIN) {
     *     Serial.println("EQXAI channel is configured as current input.");
     * }
     * @endcode
     */
    EQ_PinMode readMode(int pinIndex);


    /**
     * @brief Sets the commanded output value for an EQSP32 main-unit output pin or supported EQX expansion-module output channel.
     *
     * This function writes an output command to a pin/channel that was previously configured with `pinMode()`. The meaning and valid range of `value` depend on the configured output mode and selected hardware.
     *
     * EQSP32 main-unit outputs are open-drain / low-side pull-down outputs. In `POUT` and `RELAY` modes, the output switches the connected load to GND using PWM or ON/OFF control; it does not drive a push-pull voltage output.
     *
     * Typical value ranges:
     * - `POUT`: low-side PWM duty command, 0–1000 representing 0–100%.
     * - `RELAY`: relay power command, 0–1000. The initial power is set by `pinValue()`, then `configRELAY()` controls the drop to hold power after the derate delay.
     * - `AOUT`: analog output target voltage in millivolts, typically 0–10000 mV on supported EQX analog-output modules.
     * - `SAOUT`: synchronized analog output target voltage in millivolts, typically 0–10000 mV on supported EQX analog-output modules.
     * - Digital or relay-style EQX outputs: 0 means OFF; non-zero means ON, where supported. PWM-capable EQX outputs use the mode-specific value range.
     *
     * @param pinIndex The EQSP32 pin index or encoded EQX output channel code.
     * @param value The output command value. Valid range depends on the configured output mode/channel.
     *
     * @return `true` if the output command was accepted; `false` if the pin/channel is invalid, unavailable, not configured as a supported output, or the value is outside the allowed range.
     *
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_5, POUT);
     * eqsp32.pinValue(EQ_PIN_5, 750);              // 75% low-side PWM duty
     *
     * eqsp32.pinMode(EQ_PIN_6, RELAY);
     * eqsp32.configRELAY(EQ_PIN_6, 300, 1500);
     * eqsp32.pinValue(EQ_PIN_6, 1000);             // Start relay at full power
     *
     * eqsp32.pinMode(EQXRA(1, EQXRA_AO_1), AOUT);
     * eqsp32.pinValue(EQXRA(1, EQXRA_AO_1), 5000); // 5.000 V analog output target
     * @endcode
     */
    bool pinValue(int pinIndex, int value);


    /**
     * @brief Reads the value of an EQSP32 main-unit pin or supported EQX expansion-module channel.
     *
     * This function returns a value based on the pin/channel mode previously set with `pinMode()`. For digital input modes, the optional `trigMode` parameter can be used to detect state, rising edge, falling edge, or toggle events. For analog, sensor, counter, and output modes, `trigMode` is ignored unless otherwise documented.
     *
     * Return values by mode:
     * - `DIN` / `SWT`: digital state or trigger event, depending on `trigMode`.
     * - `PCC`: accumulated pulse count since the last read; reading clears the count.
     * - `AIN`: measured voltage in millivolts (mV).
     * - `CIN`: measured current in mA × 100, or `CIN_OC_ERROR` on overcurrent.
     * - `RAIN`: relative analog value from 0–1000, referenced to the monitored 5V VOut rail.
     * - `TIN`: temperature in Celsius × 10, or `TIN_OPEN_CIRCUIT` / `TIN_SHORT_CIRCUIT`.
     * - `PH`: pH × 100.
     * - `TC`: thermocouple temperature in Celsius × 10, or a `TC_FAULT_*` code.
     * - `PT100_24W` / `PT100_3W`: PT100 temperature in Celsius × 100, or a `PT_FAULT_*` code.
     * - Output modes such as `POUT`, `RELAY`, `AOUT`, and `SAOUT` generally return the last commanded value; feedback/status channels may return module-specific values.
     *
     * @param pinIndex The EQSP32 pin index or encoded EQX channel code to read.
     * @param trigMode Trigger mode for digital input reads. Options are `STATE`, `ON_RISING`, `ON_FALLING`, and `ON_TOGGLE`. Default is `STATE`.
     *
     * @return The pin/channel value according to its configured mode, or `-1` if the pin/channel is invalid, unavailable, or unsupported.
     *
     * @note For `PCC`, pulse trigger behavior is configured using `configPCC()`, not the `trigMode` parameter of `readPin()`.
     * @note When using edge trigger modes with digital inputs, each read updates the internal previous-state tracking. Reading with one trigger mode may affect the result of a later read using another trigger mode.
     *
     * @example
     * @code
     * int digitalState = eqsp32.readPin(EQ_PIN_1);              // DIN/SWT state
     * int risingEdge   = eqsp32.readPin(EQ_PIN_1, ON_RISING);   // Digital rising-edge event
     * int voltage_mV   = eqsp32.readPin(EQ_PIN_2);              // AIN in mV
     * int temp_raw     = eqsp32.readPin(EQ_PIN_3);              // TIN in Celsius × 10
     * int pulseCount   = eqsp32.readPin(EQ_PIN_12);             // PCC count, clears after read
     *
     * float temperature_C = CONVERT_TIN(temp_raw);
     * @endcode
     */
    int readPin(int pinIndex, EQ_TrigMode trigMode = STATE);


    /**
     * @brief Configures the PWM frequency for the Power PWM Output (POUT) mode on the EQSP32.
     * 
     * This function allows you to set the PWM frequency for the Power PWM Output (POUT) mode and all special POUT modes.
     * A change on power PWM frequency will affect all pins configured in POUT mode or any special POUT mode.
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
     * @return true if the configuration is successful, false otherwise. Returns false if the provided frequency is outside the valid range (50 Hz to 3000 Hz).
     * 
     * @note The frequency range of the power PWM output is within audible range.
     * 
     * @example
     * bool success = eqsp32.configPOUTFreq(500); // Sets the PWM frequency for Power PWM Output (POUT) mode to 500 Hz
     */
    bool configPOUTFreq(int freq);
    #define configPOUT(x)   configPOUTFreq(x)


        // Special mode configurations
    /**
     * @brief Configures the Switch (SWT) debounce settings on the EQSP32.
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
     * @note This function does not set the pin mode. Configure the pin first using `pinMode(pinIndex, SWT)`; this function only adjusts additional settings for a pin already configured in SWT mode.
     * 
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_3, SWT);
     * bool success = eqsp32.configSWT(EQ_PIN_3, 150); // Configures pin 3 for Switch (SWT) mode with a debounce time of 150 milliseconds
     * @endcode
     */
    bool configSWT(int pinIndex, int debounceTime_ms = 100);


    /**
     * @brief Configures the Temperature Input (TIN) conversion settings on the EQSP32.
     * 
     * This function allows you to configure the Temperature Input (TIN) conversion settings for a specified pin on the EQSP32.
     * The pin must already be configured in TIN mode using `pinMode(pinIndex, TIN)`.
     * In TIN mode, the pin operates as a special analog input mode designed for automatic temperature conversion.
     * You can set the beta coefficient and reference resistance values using the 'beta' and 'referenceResistance' parameters, respectively.
     * 
     * @param pinIndex The index of the pin to configure TIN conversion settings for (1 to 8).
     * @param beta The beta coefficient value for temperature conversion. Default is 3435.
     * @param referenceResistance The reference resistance value for temperature conversion, in ohms. Default is 10000.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided pin index is out of range (1 to 8), or if the pin is not already configured in TIN mode.
     *
     * @note This function does not set the pin mode.
     * Configure the pin first using `pinMode(pinIndex, TIN)`;
     * this function only adjusts additional settings for a pin already configured in TIN mode.
     * `readPin(pinIndex)` returns the converted temperature in Celsius × 10.
     * 
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_2, TIN);
     * bool success = eqsp32.configTIN(EQ_PIN_2, 4000, 12000); // Beta 4000, reference resistance 12000 ohms
     * @endcode
     */
    bool configTIN(int pinIndex, int beta = 3435, int referenceResistance = 10000);


    /**
     * @brief Configures TIN temperature conversion using Steinhart-Hart coefficients.
     *
     * Use this overload when your NTC thermistor datasheet provides standard Steinhart-Hart A, B, and C coefficients instead of a Beta value.
     *
     * The pin must already be configured in `TIN` mode. This function only sets the temperature-conversion coefficients used by `readPin(pinIndex)`.
     *
     * @param pinIndex The EQSP32 input pin to configure. Valid range is 1 to 8 for EQSP32 main-unit pins.
     * @param A The Steinhart-Hart A coefficient from the thermistor datasheet.
     * @param B The Steinhart-Hart B coefficient from the thermistor datasheet.
     * @param C The Steinhart-Hart C coefficient from the thermistor datasheet.
     *
     * @return `true` if the TIN conversion settings were applied successfully; otherwise `false`.
     *
     * @note This function does not set the pin mode.
     * Configure the pin first using `pinMode(pinIndex, TIN)`;
     * this function only adjusts additional settings for a pin already configured in TIN mode.
     * `readPin(pinIndex)` returns the converted temperature in Celsius × 10.
     *
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_2, TIN);
     * bool success = eqsp32.configTIN(EQ_PIN_2, 0.001129148, 0.000234125, 0.0000000876741);
     *
     * int temp_raw = eqsp32.readPin(EQ_PIN_2);   // Celsius × 10
     * float temp_C = CONVERT_TIN(temp_raw);
     * @endcode
     */
    bool configTIN(int pinIndex, double A, double B, double C);


    /**
     * @brief Configures TIN temperature conversion using normalized Steinhart-Hart coefficients.
     *
     * Use this overload when your NTC thermistor datasheet provides normalized Steinhart-Hart coefficients referenced to the thermistor nominal resistance.
     * This form is commonly used by some thermistor manufacturers, where the logarithmic term is based on the ratio between the measured thermistor resistance and its reference resistance at 25 °C.
     *
     * The normalized Steinhart-Hart equation is:
     *
     *     1/T = nA + nB*ln(R/Rref) + nC*(ln(R/Rref))^3
     *
     * where T is the temperature in Kelvin, R is the measured thermistor resistance in ohms, Rref is the thermistor reference resistance in ohms, typically specified at 25 °C,
     * and nA, nB, and nC are the normalized Steinhart-Hart coefficients provided by the thermistor manufacturer.
     *
     * The pin must already be configured in `TIN` mode.
     * This function only sets the normalized temperature-conversion coefficients used by `readPin(pinIndex)`.
     * `readPin(pinIndex)` returns the converted temperature in Celsius × 10.
     *
     * @param pinIndex The EQSP32 input pin to configure. Valid range is 1 to 8 for EQSP32 main-unit pins.
     * @param nA The normalized Steinhart-Hart nA coefficient from the thermistor datasheet.
     * @param nB The normalized Steinhart-Hart nB coefficient from the thermistor datasheet.
     * @param nC The normalized Steinhart-Hart nC coefficient from the thermistor datasheet.
     * @param referenceResistance The thermistor reference resistance (Rref) in ohms at 25 °C.
     *
     * @return `true` if the TIN conversion settings were applied successfully; otherwise `false`.
     *
     * @note This function does not set the pin mode. Configure the pin first using `pinMode(pinIndex, TIN)`;
     * this function only adjusts additional settings for a pin already configured in TIN mode.
     * `readPin(pinIndex)` returns the converted temperature in Celsius × 10.
     * @note This overload uses the normalized Steinhart-Hart equation based on `ln(R/Rref)`.
     * It is different from the standard Steinhart-Hart overload, which uses `ln(R)` directly and does not require a reference resistance.
     * @note Some manufacturers provide an additional D coefficient using `1/T = nA + nB*x + nC*x² + nD*x³`, where `x = ln(R/Rref)`.
     * This overload ignores the D coefficient and uses only the nA, nB, and nC terms.
     *
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_2, TIN);
     *
     * bool success = eqsp32.configTIN(
     *     EQ_PIN_2,
     *     3.354016E-03,
     *     2.569850E-04,
     *     2.620131E-06,
     *     10000
     * );
     *
     * int temp_raw = eqsp32.readPin(EQ_PIN_2);   // Celsius × 10
     * float temp_C = CONVERT_TIN(temp_raw);
     * @endcode
     */
    bool configTIN(int pinIndex, double nA, double nB, double nC, int referenceResistance);

    /**
     * @brief Configures the Relay (RELAY) output settings on the EQSP32.
     * 
     * This function allows you to configure the Relay (RELAY) mode for a specified pin on the EQSP32.
     * In RELAY mode, the pin operates as a special Power PWM Output (POUT) mode designed for relay control.
     * You can set the hold value and derate delay using the 'holdValue' and 'derateDelay' parameters, respectively.
     * When operating in RELAY mode, the initial power is set by 'pinValue' function and when the configured derateDelay
     * has passed, the power will automatically drop to the specified holdValue.
     * 
     * @param pinIndex The index of the pin to configure for Relay (RELAY) mode (1 to 16).
     * @param holdValue The hold value for relay control. This is the power level to maintain after the derate delay. Default is 500 meaning 50% of max power.
     * @param derateDelay The derate delay in milliseconds. This is the time delay before dropping to the holding power level. Default is 1000 milliseconds.
     * 
     * @return true if the configuration is successful, false otherwise. Returns false if the provided pin index is out of range (1 to 16).
     *
     * @note This function does not set the pin mode. Configure the pin first using `pinMode(pinIndex, RELAY)`; this function only adjusts additional settings for a pin already configured in RELAY mode.
     * 
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_4, RELAY);
     * bool success = eqsp32.configRELAY(EQ_PIN_4, 300, 1500); // Configures pin 4 for Relay (RELAY) mode with a hold value of 300 and a derate delay of 1500 milliseconds
     * @endcode
     */    
    bool configRELAY(int pinIndex, int holdValue = 500, int derateDelay = 1000);


    /**
     * @brief Configures the trigger behavior for a pin already set to PCC (Pulse Capture Count) mode.
     *
     * PCC mode counts input pulses on selected EQSP32 terminal pins (9–16) using hardware-level pulse counters. Before calling this function, the pin must first be configured as PCC:
     *     eqsp32.pinMode(pinIndex, PCC);
     *
     * Hardware limitations:
     * - Only terminal pins 9 through 16 support PCC mode.
     * - A maximum of four PCC pins can be active at the same time.
     * - If you attempt to configure a fifth PCC pin, the function returns `false`.
     * - The pulse count is cleared each time `readPin(pinIndex)` is called.
     *
     * Supported trigger modes:
     * - `ON_RISING` counts rising edges only. This is the default.
     * - `ON_FALLING` counts falling edges only.
     * - `ON_TOGGLE` counts both rising and falling edges.
     *
     * Any unsupported pin, unavailable PCC channel, or invalid trigger mode causes the function to return `false`.
     *
     * @param pinIndex Terminal pin index to configure. Must be between 9 and 16.
     * @param pulseCaptureMode Pulse counting trigger mode. Default is `ON_RISING`.
     * @return `true` if the PCC trigger configuration was applied successfully; otherwise `false`.
     *
     * @note This function does not set the pin mode. Configure the pin first using `pinMode(pinIndex, PCC)`; this function only adjusts additional settings for a pin already configured in PCC mode.
     * 
     * @example
     * @code
     * eqsp32.pinMode(EQ_PIN_12, PCC);
     * eqsp32.configPCC(EQ_PIN_12);              // Uses default ON_RISING
     *
     * eqsp32.pinMode(EQ_PIN_14, PCC);
     * eqsp32.configPCC(EQ_PIN_14, ON_FALLING);  // Falling edge counting
     *
     * int pulses = eqsp32.readPin(EQ_PIN_12);   // Reads and clears accumulated count
     * @endcode
     */
    bool configPCC(int pinIndex, EQ_TrigMode pulseCaptureMode = ON_RISING);


    /**
     * @brief Configures watchdog behavior for the selected supported channel.
     *
     * This function configures a watchdog-related setting for the selected channel.
     * The meaning of the watchdog value depends on the channel type.
     *
     * Typical uses include enabling/disabling a system watchdog or setting an output
     * watchdog timeout. A timeout value of `0` commonly disables the respective watchdog,
     * where supported.
     *
     * @param pinIndex Encoded channel code.
     * @param triggerDelay_ms Watchdog enable/timeout value in milliseconds.
     *                        The accepted range and exact meaning are channel dependent.
     *
     * @return `true` if the configuration was accepted,
     *         `false` if the channel does not support watchdog configuration,
     *         is not available, or the value is invalid.
     *
     * @note Unsupported channels return `false`.
     *
     * @example
     * @code
     * // EQXRA examples:
     *
     * // Enable EQXRA system/software watchdog.
     * // If the EQSP32 does not poll the EQXRA within the required time window,
     * // all relays and analog outputs are disabled.
     * eqsp32.configWDG(EQXRA(1, EQXRA_SYSSTAT), 1); // It is strongly recommended to NOT disable the system WDG.
     *
     * // Configure the shared EQXRA relay watchdog to 200 ms.
     * // Any relay channel can be used because the relay watchdog is shared.
     * // If no new relay command is received within 200 ms, all relays are disabled.
     * eqsp32.configWDG(EQXRA(1, EQXRA_RL_1), 200);
     *
     * // Configure EQXRA AO watchdogs independently.
     * // If no new AO command is received within the configured time,
     * // the respective AO channel is disabled.
     * eqsp32.configWDG(EQXRA(1, EQXRA_AO_1), 1000);
     * eqsp32.configWDG(EQXRA(1, EQXRA_AO_2), 2000);
     *
     * // Disable AO1 watchdog.
     * eqsp32.configWDG(EQXRA(1, EQXRA_AO_1), 0);
     * @endcode
     */
    bool configWDG(int pinIndex, int triggerDelay_ms);


    /**
     * @brief Configures feedback deviation monitoring for a supported channel.
     *
     * This function configures a feedback-deviation guard for a channel that supports
     * comparing a commanded value against a measured feedback value.
     *
     * The deviation threshold defines the allowed difference between the commanded value
     * and the measured feedback value. The trigger delay defines how long the deviation
     * must persist before the channel reports a deviation fault or diagnostic condition.
     *
     * Passing `0` for the threshold or delay commonly disables the respective part of
     * the check, where supported. The accepted ranges and exact behavior are channel
     * dependent.
     *
     * @param pinIndex Encoded channel code.
     * @param deviationThr_mV Allowed commanded-versus-feedback deviation threshold in mV.
     * @param triggerDelay_ms Time in milliseconds that the deviation must persist before
     *                        it is reported.
     *
     * @return `true` if the configuration was accepted,
     *         `false` if the channel does not support feedback deviation configuration,
     *         is not available, or the value is invalid.
     *
     * @note Unsupported channels return `false`.
     *
     * @example
     * @code
     * // EQXRA examples:
     *
     * // Report AO1 feedback deviation if error exceeds 300 mV for 100 ms
     * eqsp32.configFBDEV(EQXRA(1, EQXRA_AO_1), 300, 100);
     *
     * // Configure AO2 with the same feedback deviation guard
     * eqsp32.configFBDEV(EQXRA(1, EQXRA_AO_2), 300, 100);
     *
     * // Disable AO1 feedback deviation monitoring
     * eqsp32.configFBDEV(EQXRA(1, EQXRA_AO_1), 0, 0);
     * @endcode
     */
    bool configFBDEV(int pinIndex, int deviationThr_mV, int triggerDelay_ms);

    
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
     * EQSP32 eqsp32;
     * eqsp32.buzzerOn(1000, 0);  // Turns on the buzzer at 1000 Hz, remains on indefinitely
     * delay(200);
     * eqsp32.buzzerOff();        // Turns off the buzzer after 200 ms
     */
    void buzzerOff();


    /**
     * @brief Turns on the BLE communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually turns on the BLE status LED, but only if the `disableErqosIoT` flag
     * in the `EQSP32Configs` structure is set to `true`. When this flag is enabled, EQSP32's
     * internal background tasks will not control the BLE or Wi-Fi LEDs, allowing the developer
     * to reflect connection states or events manually.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing BLE LED state).
     * - The LED remains on until changed via `resetBleLed()` or `toggleBleLed()`.
     * 
     * @return `true` if the LED was successfully turned on; `false` otherwise.
     * 
     * @example
     * eqsp32.setBleLed();  // Manually turns on the BLE LED
     */
    bool setBleLed();
    #define setBLELed()     setBleLed()


    /**
     * @brief Turns off the BLE communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually turns off the BLE status LED, but only if the `disableErqosIoT` flag
     * in the `EQSP32Configs` structure is set to `true`. When this flag is enabled, EQSP32's
     * internal background tasks will not control the BLE or Wi-Fi LEDs, allowing the developer
     * to reflect connection states or events manually.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing BLE LED state).
     * - The LED remains off until changed via `setBleLed()` or `toggleBleLed()`.
     * 
     * @return `true` if the LED was successfully turned off; `false` otherwise.
     * 
     * @example
     * eqsp32.resetBleLed();  // Manually turns off the BLE LED
     */
    bool resetBleLed();
    #define resetBLELed()    resetBleLed()


    /**
     * @brief Toggles the current state of the BLE communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually toggles the BLE LED — turning it ON if it was OFF, or OFF if it was ON —
     * but only if the `disableErqosIoT` flag in the `EQSP32Configs` structure is set to `true`.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing BLE LED state).
     * - This is a single toggle action; it does not cause blinking or continuous toggling.
     * - The resulting LED state remains until changed by another `toggleBleLed()` or other LED control function.
     * 
     * @return `true` if the LED state was successfully toggled; `false` otherwise.
     * 
     * @example
     * eqsp32.toggleBleLed();  // Inverts BLE LED state (ON → OFF or OFF → ON)
     */
    bool toggleBleLed();
    #define toggleBLELed()    toggleBleLed()


    /**
     * @brief Turns on the Wi-Fi communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually turns on the Wi-Fi status LED, but only if the `disableErqosIoT` flag
     * in the `EQSP32Configs` structure is set to `true`. When this flag is enabled, EQSP32's
     * internal background tasks will not control the BLE or Wi-Fi LEDs, allowing the developer
     * to reflect connection states or events manually.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing Wi-Fi LED state).
     * - The LED remains on until changed via `resetWifiLed()` or `toggleWifiLed()`.
     * 
     * @return `true` if the LED was successfully turned on; `false` otherwise.
     * 
     * @example
     * eqsp32.setWifiLed();  // Manually turns on the Wi-Fi LED
     */
    bool setWifiLed();
    #define setWiFiLed()    setWifiLed()
    #define setWIFILed()    setWifiLed()


    /**
     * @brief Turns off the Wi-Fi communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually turns off the Wi-Fi status LED, but only if the `disableErqosIoT` flag
     * in the `EQSP32Configs` structure is set to `true`. When this flag is enabled, EQSP32's
     * internal background tasks will not control the BLE or Wi-Fi LEDs, allowing the developer
     * to reflect connection states or events manually.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing Wi-Fi LED state).
     * - The LED remains off until changed via `setWifiLed()` or `toggleWifiLed()`.
     * 
     * @return `true` if the LED was successfully turned off; `false` otherwise.
     * 
     * @example
     * eqsp32.resetWifiLed();  // Manually turns off the Wi-Fi LED
     */
    bool resetWifiLed();
    #define resetWiFiLed()      resetWifiLed()
    #define resetWIFILed()      resetWifiLed()


    /**
     * @brief Toggles the current state of the Wi-Fi communication LED if EQSP32 is not managing it internally.
     * 
     * This function manually toggles the Wi-Fi LED — turning it ON if it was OFF, or OFF if it was ON —
     * but only if the `disableErqosIoT` flag in the `EQSP32Configs` structure is set to `true`.
     * 
     * - Has no effect if `disableErqosIoT` is `false` (i.e., EQSP32 is actively managing Wi-Fi LED state).
     * - This is a single toggle action; it does not cause blinking or continuous toggling.
     * - The resulting LED state remains until changed by another `toggleWifiLed()` or other LED control function.
     * 
     * @return `true` if the LED state was successfully toggled; `false` otherwise.
     * 
     * @example
     * eqsp32.toggleWifiLed();  // Inverts Wi-Fi LED state (ON → OFF or OFF → ON)
     */
    bool toggleWifiLed();
    #define toggleWiFiLed()    toggleWifiLed()
    #define toggleWIFILed()    toggleWifiLed()


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
     * @brief Reads the monitored external 5V VOut rail voltage in millivolts.
     *
     * This function returns the voltage of the EQSP32 `5V VOut` rail, measured in millivolts (mV).
     * `5V VOut` is the external 5V output supply available for powering connected sensors or devices; it is not the USB-C 5V supply.
     *
     * The `5V VOut` rail is also used as the reference voltage for modes such as `TIN` and `RAIN`.
     *
     * @note The external `5V VOut` rail is activated only when the EQSP32 main input supply (`VIn`) is present.
     * If the EQSP32 is powered only from USB-C, and `VIn` is not supplied, the external `5V VOut` rail is not activated.
     * USB-C 5V is used internally and does not power the external `5V VOut` rail.
     * For a normal reading near 5000 mV, power the EQSP32 from `VIn` and ensure the 5V output is not overloaded or shorted.
     *
     * @return The measured external `5V VOut` rail voltage in millivolts (mV).
     *
     * @example
     * @code
     * EQSP32 eqsp32;
     *
     * int vout_mV = eqsp32.readOutputVoltage();  // External 5V VOut rail, not USB 5V
     * @endcode
     */
    int readOutputVoltage();

    /**
     * @brief Reads the state of the onboard user button (BOOT).
     * 
     * This function returns the current state of the EQSP32 onboard user (BOOT) button.
     * The button serves dual purposes depending on whether the EQSP32 IoT core
     * is enabled or disabled.
     * 
     *Behavior when EQSP32 IoT core is ENABLED (`disableErqosIoT = false`)
     * - A short press or tap enables BLE advertising, allowing discovery and
     *   configuration via the EQConnect application.
     * - A long press (press duration ≥ 3 seconds) resets all network-related
     *   parameters to factory defaults, including:
     *   - Wi-Fi SSID and password
     *   - MQTT discovery configuration
     *   - IP configuration (static IP, gateway, subnet, DNS)
     * 
     * These system-level actions are handled internally by the EQSP32 core.
     * The user application may still read the button state, but the button
     * is actively used by the system.
     * 
     *Behavior when EQSP32 IoT core is DISABLED (`disableErqosIoT = true`)
     * - The EQSP32 core does not assign any system functionality to the button.
     * - The button state is fully available to the user application.
     * - The button may be freely used for custom logic such as triggering modes,
     *   diagnostics, safety actions, or visual indicators.
     * 
     * Additional notes:
     * - The function is non-blocking.
     * - The returned state reflects the instantaneous button condition.
     * - Debouncing, press-duration tracking, or edge detection must be handled
     *   by the user in application code if required.
     * 
     * @return true if the user button is currently pressed, false otherwise.
     * 
     * @example
     * @code
     * if (eqsp32.readUserButton()) {
     *     Serial.println("User button pressed");
     * }
     * @endcode
     */
    bool readUserButton();


        // RS232/RS485 serial communication
    /**
     * @brief Configures the EQSP32 onboard serial interface mode and baud rate.
     *
     * This function configures the shared onboard serial interface for RS232 or RS485 operation.
     * Use this when you want the EQSP32 library to prepare the onboard transceiver mode for the selected serial protocol.
     *
     * Supported serial modes:
     * - `RS232`: Standard RS232 mode.
     * - `RS232_INV`: Inverted RS232 mode, for devices/signals that require inverted logic.
     * - `RS485_TX`: RS485 transmit mode.
     * - `RS485_RX`: RS485 receive mode.
     *
     * @param mode Serial interface mode: `RS232`, `RS232_INV`, `RS485_TX`, or `RS485_RX`. Default is `RS232`.
     * @param baud Serial baud rate. Default is `115200`.
     *
     * @return `true` if the serial configuration was applied successfully; otherwise `false`.
     * 
     * @attention When switching RS485 direction, pass the baud rate each time if it is not 115200.
     *
     * @example
     * @code
     * eqsp32.configSerial(RS232, 9600);
     * eqsp32.configSerial(RS485_RX, 19200);
     * @endcode
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
     * For accepting broadcast messages like NMT, configCAN() should be used to accept ALL IDs and implement software filtering
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


    /**
     * @brief Retrieves the current Wi-Fi connection status of the EQSP32 module.
     * 
     * This function returns the current Wi-Fi status, indicating whether the EQSP32 is connected, disconnected,
     * reconnecting, or scanning for available networks. The returned value corresponds to the `EQ_WifiStatus` enum.
     * 
     * The possible return values are:
     * - EQ_WF_DISCONNECTED (0): Not connected to any Wi-Fi network.
     * - EQ_WF_CONNECTED (1): Successfully connected to a Wi-Fi network.
     * - EQ_WF_RECONNECTING (2): Attempting to reconnect after disconnection.
     * - EQ_WF_SCANNING (3): Scanning for available networks.
     * 
     * @return The current Wi-Fi status as an `EQ_WifiStatus` enum.
     * 
     * @example
     * EQSP32 eqsp32;
     * EQ_WifiStatus wifiStatus = eqsp32.getWiFiStatus();
     * if (wifiStatus == EQ_WF_CONNECTED) {
     *     Serial.println("Wi-Fi is connected.");
     * } else {
     *     Serial.println("Wi-Fi is not connected.");
     * }
     */
    EQ_WifiStatus getWiFiStatus();


    /**
     * @brief Retrieves the current Ethernet connection status of the EQSP32 module.
     * 
     * This function returns the current Ethernet status of the EQSP32, indicating whether a cable is plugged in,
     * whether the Ethernet connection is established and online, or if the Ethernet interface has been stopped.
     * The returned value corresponds to the `EQ_EthernetStatus` enum.
     * 
     * The possible return values are:
     * - EQ_ETH_DISCONNECTED (0): No Ethernet cable detected or no link.
     * - EQ_ETH_CONNECTED (1): Ethernet is connected and online.
     * - EQ_ETH_PLUGGED_IN (2): Cable is detected, but connection is not yet online (e.g., IP not acquired).
     * - EQ_ETH_STOPPED (3): Ethernet interface is stopped, disabled or not detected (might be non-ethernet EQSO32 version).
     * 
     * @return The current Ethernet status as an `EQ_EthernetStatus` enum.
     * 
     * @example
     * EQSP32 eqsp32;
     * EQ_EthernetStatus ethStatus = eqsp32.getEthernetStatus();
     * if (ethStatus == EQ_ETH_CONNECTED) {
     *     Serial.println("Ethernet is online.");
     * } else {
     *     Serial.println("Ethernet is not connected or not ready.");
     * }
     */
    EQ_EthernetStatus getEthernetStatus();

    /**
     * @brief Retrieves the current MQTT broker connection status of the EQSP32 module.
     *
     * This function returns the current MQTT status of the EQSP32, indicating whether the device is connected
     * to an MQTT broker, currently attempting to connect, disconnected (and not trying), or if MQTT has not been
     * initialized/configured yet. The returned value corresponds to the `EQ_MQTTBrokerStatus` enum.
     *
     * The possible return values are:
     * - EQ_MQTT_DISCONNECTED (0): No connection with broker, not trying.
     * - EQ_MQTT_CONNECTED (1): Connected to the MQTT broker.
     * - EQ_MQTT_CONNECTING (2): Not connected yet; the system is currently trying to connect.
     * - EQ_MQTT_NO_INIT (0xFF): MQTT has not been initialized or setup.
     *
     * @return The current MQTT broker status as an `EQ_MQTTBrokerStatus` enum.
     *
     * @example
     * EQSP32 eqsp32;
     * EQ_MQTTBrokerStatus mqttStatus = eqsp32.getMQTTStatus();
     * if (mqttStatus == EQ_MQTT_CONNECTED) {
     *     Serial.println("MQTT is connected.");
     * } else if (mqttStatus == EQ_MQTT_CONNECTING) {
     *     Serial.println("MQTT is connecting...");
     * } else if (mqttStatus == EQ_MQTT_NO_INIT) {
     *     Serial.println("MQTT not initialized.");
     * } else {
     *     Serial.println("MQTT disconnected.");
     * }
     */
    EQ_MQTTBrokerStatus getMQTTStatus();

    /**
     * @brief Retrieves the current Bluetooth Low Energy (BLE) status of the EQSP32 module.
     * 
     * This function returns the current BLE state of the EQSP32, indicating whether the BLE
     * subsystem is advertising, connected to a client, subscribed to notifications, or
     * not initialized. The returned value corresponds to the `EQ_BleStatus` enum.
     * 
     * The possible return values are:
     * - EQ_BLE_DISCONNECTED (0): BLE is initialized but not connected and not advertising.
     * - EQ_BLE_CONNECTED (1): BLE is connected to a client, but no active notification
     *   subscriptions are present.
     * - EQ_BLE_ADVERTISING (2): BLE is advertising and discoverable by nearby BLE devices.
     * - EQ_BLE_SUBSCRIBED (3): BLE is connected to a client and the client has subscribed
     *   to notification streams.
     * - EQ_BLE_NO_INIT (255): BLE has not been initialized or is not managed by the EQSP32
     *   library (for example when Erqos IoT services are disabled).
     * 
     * @return The current BLE status as an `EQ_BleStatus` enum.
     * 
     * @example
     * EQ_BleStatus bleStatus = eqsp32.getBleStatus();
     * 
     * if (bleStatus == EQ_BLE_ADVERTISING) {
     *     Serial.println("BLE advertising and ready for connection.");
     * } else if (bleStatus == EQ_BLE_CONNECTED) {
     *     Serial.println("BLE client connected.");
     * } else if (bleStatus == EQ_BLE_SUBSCRIBED) {
     *     Serial.println("BLE notifications active.");
     * } else if (bleStatus == EQ_BLE_DISCONNECTED) {
     *     Serial.println("BLE initialized but not connected.");
     * } else if (bleStatus == EQ_BLE_NO_INIT) {
     *     Serial.println("BLE not initialized.");
     * }
     */
    EQ_BleStatus getBleStatus();


    // WiFi control
    /**
     * @brief Enables Wi-Fi connectivity on the EQSP32.
     * 
     * This function attempts to start Wi-Fi and connect the EQSP32 to a wireless
     * network.
     * 
     * This function is intended for use when Erqos IoT handling is enabled,
     * allowing the application to control Wi-Fi while the EQSP32 library
     * manages the connection workflow internally.
     * 
     * If Erqos IoT handling is disabled and custom Wi-Fi management is used
     * instead, this function is not used and always returns `false`.
     * 
     * If `newSSID` and `newPASS` are left empty, the function tries to connect
     * using the Wi-Fi credentials currently stored in flash memory, if available.
     * 
     * If `newSSID` and `newPASS` are provided, they overwrite any previously
     * stored Wi-Fi credentials and the EQSP32 attempts to connect using the new
     * values.
     * 
     * The function returns `false` if Wi-Fi cannot begin a connection attempt,
     * such as when no valid credentials are available or another condition is
     * preventing Wi-Fi startup.
     * 
     * @param newSSID Optional new Wi-Fi SSID to use and store. If left empty,
     *                the EQSP32 uses the saved SSID & password from flash memory.
     * @param newPASS Optional new Wi-Fi password to use and store. If left empty,
     *                the EQSP32 uses the saved SSID & password from flash memory.
     * 
     * @return `true` if Wi-Fi was successfully allowed to start and is attempting
     *         to connect, `false` if Wi-Fi could not begin the connection process.
     * 
     * @example
     * if (eqsp32.startWiFi()) {
     *     Serial.println("Wi-Fi is starting with saved credentials.");
     * } else {
     *     Serial.println("Wi-Fi could not start.");
     * }
     * 
     * @example
     * if (eqsp32.startWiFi("MySSID", "MyPassword")) {
     *     Serial.println("Wi-Fi is starting with new credentials.");
     * } else {
     *     Serial.println("Wi-Fi could not start with the provided credentials.");
     * }
     */
    bool startWiFi(String newSSID = "", String newPASS = "");
    #define startWIFI() startWiFi()

    /**
     * @brief Disables Wi-Fi connectivity on the EQSP32.
     * 
     * This function attempts to stop Wi-Fi and disconnect the EQSP32 from the
     * currently active wireless network.
     * 
     * This function is intended for use when Erqos IoT handling is enabled,
     * allowing the application to control Wi-Fi while the EQSP32 library
     * manages the connection workflow internally.
     * 
     * If Erqos IoT handling is disabled and custom Wi-Fi management is used
     * instead, this function is not used and always returns `false`.
     * 
     * If Wi-Fi is currently active, the function attempts to disconnect it and
     * stop the Wi-Fi interface.
     * 
     * @return `true` if Wi-Fi was successfully disconnected and stopped,
     *         `false` if Wi-Fi was not active or could not be stopped.
     * 
     * @example
     * if (eqsp32.stopWiFi()) {
     *     Serial.println("Wi-Fi stopped successfully.");
     * } else {
     *     Serial.println("Wi-Fi was not active or could not be stopped.");
     * }
     */
    bool stopWiFi();
    #define stopWIFI() stopWiFi()

    /**
     * @brief Clears the stored Wi-Fi credentials on the EQSP32.
     * 
     * This function deletes the Wi-Fi credentials currently stored in flash
     * memory.
     * 
     * This function is intended for use when Erqos IoT handling is enabled,
     * allowing the application to control Wi-Fi while the EQSP32 library
     * manages the connection workflow internally.
     * 
     * If Erqos IoT handling is disabled and custom Wi-Fi management is used
     * instead, this function is not used and always returns `false`.
     * 
     * Clearing Wi-Fi credentials automatically stops Wi-Fi and disconnects the
     * EQSP32 from the current wireless network, if Wi-Fi is active.
     * 
     * After clearing the stored credentials, the EQSP32 resets the Wi-Fi
     * credentials to the default values defined in `EQSP32Configs`, which may
     * be empty if no developer defaults were provided.
     * 
     * @return `true` if the stored Wi-Fi credentials were successfully cleared,
     *         `false` if the credentials could not be cleared.
     * 
     * @example
     * if (eqsp32.clearWiFi()) {
     *     Serial.println("Wi-Fi credentials cleared.");
     * } else {
     *     Serial.println("Wi-Fi credentials could not be cleared.");
     * }
     */
    bool clearWiFi();
    #define clearWIFI() clearWiFi()


    // BLE control
    /**
     * @brief Enables Bluetooth Low Energy (BLE) advertising on the EQSP32.
     * 
     * This function enables BLE advertising, allowing nearby BLE clients
     * (such as the EQConnect mobile application) to discover and connect
     * to the EQSP32.
     * 
     * If BLE is already advertising, connected, or has an active client
     * subscription, the function does not restart advertising and returns `false`.
     * 
     * @return `true` if BLE was ready and successfully started advertising,
     *         `false` if BLE was already advertising, connected, subscribed,
     *         or could not start advertising.
     * 
     * @example
     * if (eqsp32.startBle()) {
     *     Serial.println("BLE started and advertising.");
     * } else {
     *     Serial.println("BLE was already active or failed to start.");
     * }
     */
    bool startBle();
    #define startBLE() startBle()


    /**
     * @brief Stops Bluetooth Low Energy (BLE) advertising on the EQSP32.
     * 
     * This function stops BLE advertising and makes the EQSP32 no longer
     * discoverable over BLE.
     * 
     * For advertising to stop successfully, BLE must currently be in the
     * `EQ_BLE_ADVERTISING` state. If there is an active BLE connection or
     * an active client subscription, the function returns `false` and BLE
     * remains active.
     * 
     * After stopping BLE advertising, the BLE status transitions from
     * `EQ_BLE_ADVERTISING` to `EQ_BLE_DISCONNECTED`.
     * 
     * @return `true` if BLE advertising was successfully stopped,
     *         `false` if BLE was not advertising, was not initialized,
     *         had an active connection/subscription, or could not be stopped.
     * 
     * @example
     * if (eqsp32.stopBle()) {
     *     Serial.println("BLE advertising stopped.");
     * } else {
     *     Serial.println("BLE was not advertising or could not be stopped.");
     * }
     */
    bool stopBle();
    #define stopBLE() stopBle()

    /**
     * @brief Updates MQTT credentials and enable/disable behavior, persisting settings to flash.
     *
     * This function updates the MQTT broker authentication credentials (username/password) and the MQTT enabled
     * state, and stores them in flash memory. The stored values are automatically applied after reset and will be
     * used for subsequent MQTT connection attempts.
     *
     * Username and password are allowed to be empty strings, to support brokers that do not require authentication.
     *
     * @important
     * - Any values provided for `mqttUserName` and `mqttPassword` will overwrite the currently stored credentials.
     * - If the developer does not want to overwrite the stored credentials, they must pass the existing credentials.
     * - If only the enabled state needs to change, use the overloaded function `updateMQTT(bool enableMqtt)`
     *   to avoid modifying stored credentials.
     *
     * The `enableMqtt` flag is also stored in flash and applied after reset. When enabled, the EQSP32 will attempt
     * to connect to the configured MQTT broker. When disabled, the EQSP32 will ignore MQTT and will not attempt
     * connections.
     *
     * @param mqttUserName MQTT username to store (may be empty).
     * @param mqttPassword MQTT password to store (may be empty).
     * @param enableMqtt   Enables or disables MQTT auto-connection behavior (persisted to flash).
     *
     * @example
     * // Enable MQTT with credentials:
     * eqsp32.updateMQTT("user", "pass", true);
     *
     * @example
     * // Enable MQTT without authentication:
     * eqsp32.updateMQTT("", "", true);
     *
     * @example
     * // Disable MQTT without changing credentials (preferred):
     * eqsp32.updateMQTT(false);
     *
     * @example
     * // Keep existing credentials but enable MQTT (pass existing values explicitly):
     * eqsp32.updateMQTT(existingUser, existingPass, true);
     */
    void updateMQTT(String mqttUserName, String mqttPassword, bool enableMqtt);

    /**
     * @brief Enables or disables MQTT behavior, persisting the setting to flash.
     *
     * This overload updates only the MQTT enabled state without modifying stored MQTT credentials.
     * The `enableMqtt` value is stored in flash and applied after reset. When enabled, the EQSP32 will
     * attempt MQTT connections; when disabled, it will ignore MQTT and will not attempt to connect.
     *
     * @param enableMqtt Enables (`true`) or disables (`false`) MQTT auto-connection behavior (persisted to flash).
     */
    void updateMQTT(bool enableMqtt);
    

    /**
     * @brief Returns the current IP address of the EQSP32 device.
     *
     * If the device is not online (neither Ethernet nor Wi-Fi connected), the function returns "0.0.0.0".
     * If the device is online, it returns the IP address of the active interface:
     * - Ethernet IP address if Ethernet is connected.
     * - Wi-Fi IP address if Wi-Fi is connected and Ethernet is not.
     *
     * @return String representing the device's current IP address, or "0.0.0.0" if offline.
     */
    String localIP();


    /**
     * @brief Returns the current Ethernet IP address of the EQSP32 device.
     *
     * If the device is connected to Ethernet, this function returns its assigned
     * Ethernet IP address as a String.
     * 
     * If Ethernet is not connected, the function returns "0.0.0.0", even if the device
     * is online via Wi-Fi.
     *
     * @return String representing the Ethernet IP address, or "0.0.0.0" if Ethernet is not connected.
     */
    String ethernetIP();


    /**
     * @brief Returns the current Wi-Fi IP address of the EQSP32 device.
     *
     * If the device is connected to Wi-Fi, this function returns its assigned
     * Wi-Fi IP address as a String.
     * 
     * If Wi-Fi is not connected, the function returns "0.0.0.0", even if the device
     * is online via Ethernet.
     *
     * @return String representing the Wi-Fi IP address, or "0.0.0.0" if Wi-Fi is not connected.
     */
    String wifiIP();


    /**
     * @brief Checks if the EQSP32 device is currently online via either Wi-Fi or Ethernet.
     * 
     * This function determines whether the EQSP32 module is currently connected to a network.
     * It returns true if either the Wi-Fi interface is connected (`EQ_WF_CONNECTED`) or the Ethernet interface
     * is connected and online (`EQ_ETH_CONNECTED`). If neither interface is connected, it returns false.
     * 
     * This is useful for checking network availability before attempting operations that require
     * internet access.
     * 
     * @return true if either Wi-Fi or Ethernet is connected and online, false otherwise.
     * 
     * @example
     * EQSP32 eqsp32;
     * if (eqsp32.isDeviceOnline()) {
     *     Serial.println("Device is online.");
     * } else {
     *     Serial.println("Device is offline.");
     * }
     */
    bool isDeviceOnline();


    /**
     * @brief Retrieves the user-defined name of the EQSP32 device.
     * 
     * This function returns the current name assigned to the EQSP32 device by the end user.
     * The name is stored persistently and used across various services and interfaces for
     * identification and presentation.
     * 
     * The device name is internally applied in:
     * - BLE advertising and identification (takes effect only after reboot)
     * - Web server hostname (access via `http://DeviceName.home:8000`, where spaces from DeviceName are replaced with dashes and special characters are removed)
     * - Arduino OTA (used as the OTA target name)
     * - Home Assistant auto-discovery (used as the device name in the discovered MQTT device)
     * 
     * @note This name is editable by the end user at any time from EQConnect.
     * 
     * @return The user-defined device name as an Arduino `String`.
     * 
     * @example
     * EQSP32 eqsp32;
     * String name = eqsp32.getDeviceName();
     * Serial.print("Device Name: ");
     * Serial.println(name);
     */
    String getDeviceName();

    
    /**
     * @brief Sets the user-defined device name of the EQSP32.
     * 
     * This function validates and applies a new device name for the EQSP32.
     * 
     * If the new name is valid, it is saved internally and the required system
     * updates are triggered, such as persistence to flash memory and refresh of
     * services that use the device name, for example the OTA host name.
     * 
     * If the provided name is the same as the current one, the function returns
     * `true` without making further changes.
     * 
     * @param newDeviceName The new device name to validate and apply.
     * 
     * @return `true` if the device name was accepted,
     *         `false` if the device name was rejected.
     * 
     * @example
     * if (eqsp32.setDeviceName("Boiler Room Controller")) {
     *     Serial.println("Device name updated.");
     * } else {
     *     Serial.println("Invalid device name.");
     * }
     */
    bool setDeviceName(String newDeviceName);


    /**
     * @brief Retrieves the generated Device ID used by the system.
     * 
     * This function returns the current Device ID used internally for
     * MQTT topic generation and system identification.
     * 
     * The Device ID is generated automatically using the device name
     * configured via EQConnect and follows this format:
     * 
     *     EQ_{sanitized_device_name}
     * 
     * Where:
     * - The prefix "EQ_" is added automatically.
     * - Spaces in the device name are replaced with underscores (_).
     * - Special characters are removed.
     * 
     * Example:
     * If the configured device name is:
     *     My EQSP32!
     * 
     * The generated Device ID becomes:
     *     EQ_My_EQSP32
     * 
     * The Device ID is persistent across reboots but will change
     * if the device name is modified in EQConnect.
     * 
     * This ID is internally used in MQTT topic generation and may
     * also be used by developers for external integrations.
     * 
     * @return The generated Device ID as an Arduino `String`.
     * 
     * @example
     * EQSP32 eqsp32;
     * String id = eqsp32.getDeviceID();
     * Serial.print("Device ID: ");
     * Serial.println(id); // Example output: EQ_My_EQSP32
     */
    String getDeviceID();



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

    /**
     * @brief Retrieves the number of full days the EQSP32 has been running since power-up.
     * 
     * This function returns the number of whole days (24-hour periods) that have passed
     * since the EQSP32 was last powered on or reset.
     * 
     * It is based on the internal uptime counter and is useful for monitoring
     * system runtime, debugging, and diagnostics.
     * 
     * @return The uptime in full days as an `int`.
     * 
     * @example
     * EQSP32 eqsp32;
     * int days = eqsp32.getUptimeDays();
     * Serial.print("Uptime (days): ");
     * Serial.println(days);
     */
    int getUptimeDays();


    /**
     * @brief Retrieves the current hour value (0–23) from the uptime counter.
     * 
     * This function returns the number of full hours that have passed
     * since the last full 24-hour period (i.e., the hours within the current uptime day).
     * 
     * For example, if the device has been running for 1 day and 5 hours,
     * this function will return `5`.
     * 
     * It is useful for building a full uptime breakdown in days, hours, minutes, and seconds.
     * 
     * @return The number of uptime hours within the current day (0–23).
     * 
     * @example
     * EQSP32 eqsp32;
     * int h = eqsp32.getUptimeHours();
     * Serial.print("Hours today: ");
     * Serial.println(h);
     */
    int getUptimeHours();


    /**
     * @brief Retrieves the current minute value (0–59) from the uptime counter.
     * 
     * This function returns the number of full minutes that have passed
     * since the last full hour of uptime (i.e., the minutes within the current uptime hour).
     * 
     * For example, if the device has been running for 1 day, 5 hours, and 42 minutes,
     * this function will return `42`.
     * 
     * It is useful for building a complete uptime display in days, hours, minutes, and seconds.
     * 
     * @return The number of uptime minutes within the current hour (0–59).
     * 
     * @example
     * EQSP32 eqsp32;
     * int m = eqsp32.getUptimeMinutes();
     * Serial.print("Minutes this hour: ");
     * Serial.println(m);
     */
    int getUptimeMinutes();


    /**
     * @brief Retrieves the current second value (0–59) from the uptime counter.
     * 
     * This function returns the number of full seconds that have passed
     * since the last full minute of uptime (i.e., the seconds within the current uptime minute).
     * 
     * For example, if the device has been running for 1 day, 5 hours, 42 minutes, and 17 seconds,
     * this function will return `17`.
     * 
     * It is useful for building a complete uptime display in days, hours, minutes, and seconds.
     * 
     * @return The number of uptime seconds within the current minute (0–59).
     * 
     * @example
     * EQSP32 eqsp32;
     * int s = eqsp32.getUptimeSeconds();
     * Serial.print("Seconds this minute: ");
     * Serial.println(s);
     */
    int getUptimeSeconds();


    /**
     * @brief Prints the system uptime in a human-readable format.
     * 
     * This function retrieves the number of days, hours, minutes, and seconds
     * the EQSP32 has been running since power-up, and prints it to the default `Serial` output.
     * 
     * Format:
     * 
     *     Uptime: <days>d <hours>h <minutes>m <seconds>s
     * 
     * Example output:
     * 
     *     Uptime: 3d 12h 45m 8s
     * 
     * Useful for quick debugging, logging, or diagnostics from the serial console.
     * 
     * @note Output is printed to the global `Serial` object
     * 
     * @example
     * EQSP32 eqsp32;
     * eqsp32.printUptime();
     */
    void printUptime();


    /**
     * @brief Retrieves the current local time as a Unix timestamp.
     * 
     * This function returns the current local time (with timezone offset applied) as a Unix timestamp,
     * representing the number of seconds that have passed since January 1, 1970 (UTC).
     * 
     * It uses the EQSP32's internal timekeeping system, which is typically synchronized via NTP
     * if internet access is available. If the time is not synchronized, the returned value may be inaccurate
     * and based on the device's unsynchronized internal clock or default time after reboot.
     * 
     * This is useful for timestamping events, logging, scheduling tasks, or synchronizing with external systems.
     * 
     * @return The local time as a Unix timestamp (`long`, in seconds since Jan 1, 1970).
     * 
     * @example
     * EQSP32 eqsp32;
     * long ts = eqsp32.getLocalUnixTimestamp();
     * Serial.print("Current local timestamp: ");
     * Serial.println(ts);
     */
    long getLocalUnixTimestamp();


    /**
     * @brief Retrieves the current local time as a formatted string.
     * 
     * This function returns the local time as a human-readable string in the format:
     * 
     *     "MM-DD-YYYY HH:MM:SS"
     * 
     * The local time includes timezone offset and is typically synchronized via NTP when internet access is available.
     * If time synchronization has not occurred, the returned time may reflect the default RTC time since power-up.
     * 
     * This function is useful for logging, display on dashboards, or sending human-readable timestamps
     * over MQTT or to external systems.
     * 
     * @return The formatted local time as a `std::string` (in the format "MM-DD-YYYY HH:MM:SS").
     * 
     * @example
     * EQSP32 eqsp32;
     * std::string timeStr = eqsp32.getFormattedLocalTime();
     * Serial.print("Current local time: ");
     * Serial.println(timeStr.c_str());
     */
    std::string getFormattedLocalTime();


    /**
     * @brief Retrieves the current system time as a Unix timestamp (UTC).
     * 
     * This function returns the current system time in UTC (Coordinated Universal Time)
     * as a Unix timestamp — the number of seconds since January 1, 1970, 00:00:00 UTC.
     * 
     * Unlike `getLocalUnixTimestamp()`, this value does not include any timezone or daylight savings offset.
     * It reflects the raw system clock time, which is typically synchronized via NTP if internet access is available.
     * 
     * If time synchronization has not yet occurred, the result will be based on the default time of the internal RTC counter since boot.
     * 
     * @return The current UTC timestamp as a `long` (seconds since Jan 1, 1970).
     * 
     * @example
     * EQSP32 eqsp32;
     * long utc = eqsp32.getUnixTimestamp();
     * Serial.print("Current UTC timestamp: ");
     * Serial.println(utc);
     */
    long getUnixTimestamp();

    /**
     * @brief Retrieves the current UTC time as a formatted string.
     * 
     * This function returns the current UTC time in a human-readable string format:
     * 
     *     "MM-DD-YYYY HH:MM:SS"
     * 
     * This is useful for logging or display purposes where timestamps need to be
     * shown in Coordinated Universal Time, unaffected by local timezone offsets.
     * 
     * If time synchronization has not occurred, the result will reflect the default system time since power-up.
     * 
     * @return The formatted UTC time as a `std::string` (in the format "MM-DD-YYYY HH:MM:SS").
     * 
     * @example
     * EQSP32 eqsp32;
     * std::string utcStr = eqsp32.getFormattedUnixTimestamp();
     * Serial.print("UTC time: ");
     * Serial.println(utcStr.c_str());
     */
    std::string getFormattedUnixTimestamp();

private:
    class EQ_Private;       // Forward declaration of the nested private class
    EQ_Private* eqPrivate;
};

#if ESP_ARDUINO_VERSION_MAJOR > 2
#define Serial Serial0
#endif

/**
 * @brief EQTimer class for non-blocking timing operations.
 * 
 * EQTimer provides a lightweight elapsed-time timer for periodic actions,
 * timeouts, delays, and debounce-style logic without blocking the main loop.
 * 
 * A timer has an optional preset value in milliseconds. The preset is used by
 * isElapsed() and isExpiredDisabled() to evaluate timer state.
 * 
 */
class EQTimer {
public:
    /**
     * @brief Constructs an EQTimer object with an optional preset value.
     * 
     * Creates a stopped timer and stores the optional preset value. The timer
     * does not begin counting until start() is called.
     * 
     * @param preset Optional preset value in milliseconds. Default is 0.
     * 
     * @example
     * @code
     * EQTimer myTimer(5000); // Creates a stopped timer with a 5 second preset
     * @endcode
     */
    EQTimer(unsigned long preset = 0);

    /**
     * @brief Starts or resumes the timer.
     * 
     * Starts the timer if it is not already running.
     * If the timer was paused, timing resumes from the previously accumulated elapsed time.
     * 
     * If a non-zero preset value is provided, it updates the timer preset before starting or resuming.
     * Passing 0 or leaving the argument empty keeps the existing preset unchanged.
     * 
     * If resumed from pause with a new preset, the existing elapsed time is kept and compared against the new preset.
     * If the new preset is lower than or equal to the elapsed time, isExpired() will return true immediately.
     * 
     * @param preset Optional new preset value in milliseconds.
     *               Passing 0 or leaving the argument empty keeps the current preset.
     * @return true if the timer started or resumed successfully, false if it was already running.
     */
    bool start(unsigned long preset = 0);

    /**
     * @brief Stops the timer and clears elapsed time.
     * 
     * Stops the timer and resets the elapsed time to 0. The stored preset value is not changed.
     * Starting the timer again with preset 0 or no argument uses the stored preset value.
     * 
     * @example
     * @code
     * EQTimer myTimer(5000);
     * myTimer.start();
     * delay(2000);
     * myTimer.stop(); // Timer is stopped and elapsed time is cleared
     * @endcode
     */ 
    void stop();

    /**
     * @brief Pauses the timer.
     * 
     * Pauses the timer and preserves the elapsed time accumulated so far.
     * While paused, elapsed time does not increase.
     * Calling start() resumes the timer from the paused elapsed value.
     * 
     * Calling start() with a new non-zero preset after pause keeps the elapsed time and uses the new preset.
     * If the new preset is lower than or equal to the elapsed time, isExpired() will return true immediately.
     * 
     * Calling pause() when the timer is not running has no effect.
     * 
     * @example
     * @code
     * EQTimer myTimer(5000);
     * 
     * myTimer.start();
     * delay(3000);
     * myTimer.pause();
     * myTimer.start(2000); // Resumes with about 3000 ms elapsed, so isExpired() is immediately true
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
     * @brief Checks whether a running timer has elapsed.
     * 
     * Returns true only when the timer is running, has a non-zero preset, and the elapsed time has reached or exceeded the preset.
     * Returns false if the timer is stopped, paused, has no preset, or has not reached the preset yet.
     * 
     * If autoReset is true, the timer is reset immediately after an elapsed condition is detected.
     * This is useful for periodic actions that should run once per timer interval.
     * 
     * @param autoReset When true, automatically resets the timer after returning true. Default is false.
     * @return true if the running timer has elapsed, false otherwise.
     * 
     * @example
     * @code
     * EQTimer sampleTimer(1000);
     * 
     * void setup() {
     *     sampleTimer.start();
     * }
     * 
     * void loop() {
     *     if (sampleTimer.isElapsed(true)) {
     *         readSensors(); // Runs once every 1000 ms
     *     }
     * }
     * @endcode
     */
    bool isElapsed(bool autoReset = false);


    /**
     * @brief Checks whether the timer is expired or disabled.
     * 
     * When the timer is running and has a non-zero preset, this returns true once the elapsed time reaches or exceeds the preset.
     * The timer continues running after it expires until stop(), pause(), or reset() is called.
     * 
     * If the timer is stopped, paused, or has no preset value set, it is considered disabled and this function returns true.
     * Use isRunning() && isExpired() when you need to detect a real active timer expiration.
     * 
     * @return true if the timer has expired or is disabled.
     *         Returns false only while a running timer is still below its preset value.
     * 
     * @example
     * @code
     * EQTimer myTimer(3000);
     * myTimer.start();
     * 
     * if (myTimer.isRunning() && myTimer.isExpired()) {
     *     myTimer.reset(); // Real active timer expiration
     * }
     * @endcode
     */
    bool isExpiredDisabled();       // Preferred name (previously known as isExpired())
    bool isExpired();               // Deprecated compatibility alias


    /**
     * @brief Checks if the timer is currently running.
     * 
     * Determines if the timer is active and counting time. The timer will continue running even after reaching
     * its preset value, and this method will return true as long as the timer has not been stopped or paused.
     * 
     * @return Returns true if the timer is running, otherwise returns false.
     * 
     * @example
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

/** @cond INTERNAL */
protected:
    virtual unsigned long virtualElapsedTime();
    virtual unsigned long virtualMillis();
/** @endcond */

private:
    unsigned long startMillis;
    unsigned long presetValue;
    unsigned long elapsedTime;
    unsigned long virtualStartMillis;
    bool running;
};


/*  ***********************************************
     ---   ---   ---   ---   ---   ---   ---   ---
    MQTT Device Interfacing Entities for Home Assistant (Node-RED or other MQTT platforms)
     ---   ---   ---   ---   ---   ---   ---   ---
    ***********************************************   */

struct SwitchEntityConfig {
    const char* entity;     /* "Unnamed" */
    const char* icon;       /* "" */

    constexpr SwitchEntityConfig(const char* entity_ = "Unnamed",
                                const char* icon_ = "")
        : entity(entity_), icon(icon_) {}
};

struct ValueEntityConfig {
    const char* entity;     /* "Unnamed" */
    int32_t min_value;      /* 0*/
    int32_t max_value;      /* 1000 */
    uint8_t decimals;       /* 0 */
    const char* icon;       /* "" */

    constexpr ValueEntityConfig(const char* entity_ = "Unnamed",
                                int32_t min_value_ = 0,
                                int32_t max_value_ = 1000,
                                uint8_t decimals_ = 0,
                                const char* icon_ = "")
        : entity(entity_), min_value(min_value_), max_value(max_value_), decimals(decimals_), icon(icon_) {}
};

struct TextEntityConfig {
    const char* entity;      /* "Unnamed" */
    int32_t min_length;      /* 0 */
    int32_t max_length;      /* 32 */
    bool hidden_text;        /* false */
    const char* pattern;     /* "" */
    const char* icon;        /* "" */

    constexpr TextEntityConfig(const char* entity_ = "Unnamed",
                               int32_t min_length_ = 0,
                               int32_t max_length_ = 32,
                               bool hidden_text_ = false,
                               const char* pattern_ = "",
                               const char* icon_ = "")
        : entity(entity_), min_length(min_length_), max_length(max_length_), hidden_text(hidden_text_), pattern(pattern_), icon(icon_) {}
};

struct BinarySensorEntityConfig {
    const char* entity;         /* "Unnamed" */
    const char* icon;           /* "" */
    const char* device_class;   /* "" */

    constexpr BinarySensorEntityConfig(const char* entity_ = "Unnamed",
                                        const char* icon_ = "",
                                        const char* device_class_ = "")
        : entity(entity_), icon(icon_), device_class(device_class_) {}
};

struct SensorEntityConfig {
    const char* entity;         /* "Unnamed" */
    int32_t decimals;           /* 0 */
    const char* unit;           /* "" */
    const char* icon;           /* "" */
    const char* device_class;   /* "" */

    constexpr SensorEntityConfig(const char* entity_ = "Unnamed",
                                int32_t decimals_ = 0,
                                const char* unit_ = "",
                                const char* icon_ = "",
                                const char* device_class_ = "")
        : entity(entity_), decimals(decimals_), unit(unit_), icon(icon_), device_class(device_class_) {}
};

/**
 * 
 *      Control and Display entities for Home Assistant integration over MQTT
 *      {These entities will read/write MQTT topics which may be used in other integration like Node-RED}
 * 
 */
        //  ------------------------
        //      Control entities
        //  ------------------------
void createControl_Switches(const SwitchEntityConfig* items, size_t count);
void createControl_Values(const ValueEntityConfig* items, size_t count);
void createControl_Texts(const TextEntityConfig* items, size_t count);
    // Using the above {createControl_Switches() and createControl_Values()} is highly recommended, avoid using the following functions for more than 20 entities
void createControl_Switch(std::string entityName, std::string iconType_HA = "");
void createControl_Value(std::string entityName, int minValue = 0, int maxValue = 1000, int decimals = 0, std::string iconType_HA = "");
// void createControl_Text( ... );      // This is only supported via the TextEntityConfig array method

bool readControl_Switch(const std::string& entityName);
float readControl_Value(const std::string& entityName);
String readControl_Text(const std::string& entityName);

bool updateControl_Switch(const std::string& entityName, bool value, bool retain = false);
bool updateControl_Value(const std::string& entityName, float value, bool retain = false);
bool updateControl_Text(const std::string& entityName, String value, bool retain = false);


        //  ------------------------
        //      Monitoring entities
        //  ------------------------
void createDisplay_BinarySensors(const BinarySensorEntityConfig* items, size_t count);
void createDisplay_Sensors(const SensorEntityConfig* items, size_t count);
// Using the above {createDisplay_BinarySensors() and createDisplay_Sensors()} is highly recommended, avoid using the following functions for more than 20 entities

// binSensorType_HA (which could be omitted) could be one of the binary sensor types listed at https://www.home-assistant.io/integrations/binary_sensor/
void createDisplay_BinarySensor(std::string entityName, std::string iconType_HA = "", std::string binSensorType_HA = "");
// sensorType_HA (which could be omitted) could be one of the sensor types listed at https://www.home-assistant.io/integrations/sensor/
void createDisplay_Sensor(std::string entityName, int decimals = 0, std::string unit = "", std::string iconType_HA = "", std::string sensorType_HA = "");

bool readDisplay_BinarySensor(const std::string& entityName);
float readDisplay_Sensor(const std::string& entityName);

bool updateDisplay_BinarySensor(const std::string& entityName, bool value, bool retain = false);
bool updateDisplay_Sensor(const std::string& entityName, float value, bool retain = false);

        //  ------------------------------------------------------------------------
        //      Global MQTT topics (Plain MQTT topics. No UID used for these.)
        //  ------------------------------------------------------------------------
void createGlobal_Topic(const String& topic);               // Global topic registration

String readGlobal_Topic(const String& topicName);           // Read raw received topic payload as String
bool   readGlobal_TopicBOOL(const String& topicName);       // Auto conversion to true/false
float  readGlobal_TopicFLOAT(const String& topicName);      // Will try to convert the value to float

bool updateGlobal_Topic(const String& topicName, const String& payload, bool retain = false);

#endif