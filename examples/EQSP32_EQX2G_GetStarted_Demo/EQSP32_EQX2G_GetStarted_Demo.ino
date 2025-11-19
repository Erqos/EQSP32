/**
 * @file EQSP32_EQX2G_GetStarted_Demo.ino
 * @brief Basic EQX2G (SIM800) bring-up with modem health check and interactive AT console.
 *
 * This example demonstrates how to:
 * - Initialize the EQSP32 core and the EQX2G 2G (GSM/GPRS) expansion module.
 * - Verify basic modem communication using `AT` and a SIM PIN (if required).
 * - Read and display SIM status and RF signal quality (CSQ) in both raw units and dBm.
 * - Interactively send custom AT commands to the modem from the Serial Monitor.
 *
 * Operation:
 * - On startup, the sketch:
 *   - Calls `eqsp32.begin()` to initialize the EQSP32 platform.
 *   - Uses `eqx2g.testAT()` to check if the modem responds to `AT`.
 *   - Calls `eqx2g.init(EQX2G_SIM_PIN)` to unlock/initialize the SIM (if a PIN is configured).
 *   - Reads SIM status via `eqx2g.getSimStatus()` and prints whether the SIM is READY.
 *   - Optionally shows signal quality using `eqx2g.getSignalQuality()` via the `c` command.
 * - After initialization, a simple Serial menu is available:
 *   - `h` → Print this help menu again.
 *   - `s` → Re-run the modem initialization / health check.
 *   - `c` → Show current signal strength (CSQ + RSSI in dBm with a quality label).
 *   - `n` → Show network/operator info (operator name and registration status).
 *   - `t` → Enter AT console mode.
 * - In AT console mode:
 *   - Anything you type in the Serial Monitor (ending with Enter) is sent as an AT command.
 *   - A line that starts with a single `t` or `T` (and then Enter) exits AT console mode back to the menu instead of being forwarded.
 *   - `\r` characters from the Serial Monitor are ignored, and `\n` terminates the command line.
 *
 * Features:
 * - Uses the EQX2G expansion module (SIM800-based) through the EQSP32 EQX2G stream (`eqx2gStream`).
 * - Performs a minimal but useful modem health check (AT response + SIM READY).
 * - Provides human-readable RF signal quality (CSQ → dBm, with qualitative description).
 * - Offers a simple, line-based AT console for manual testing and troubleshooting.
 * - Clean, non-blocking main loop with a small delay to keep FreeRTOS tasks responsive.
 *
 * Requirements:
 * - EQSP32 controller with the EQSP32 library correctly installed and configured.
 * - EQX2G 2G GSM/GPRS expansion module connected to the EQSP32 expansion port.
 * - SIM card inserted in the EQX2G:
 *   - If the SIM requires a PIN, set `EQX2G_SIM_PIN` to the correct PIN.
 *   - If no PIN is required, set `EQX2G_SIM_PIN` to an empty string "" and adjust the code accordingly.
 * - TinyGSM library and the EQX2G stream wrapper (`eqx2gStream`) properly integrated in the project.
 * - USB connection to a PC with a Serial Monitor:
 *   - Baud rate: 115200
 *   - Line ending: typically "Newline" (`\n`) or "Both NL & CR" is recommended.
 *
 * Notes:
 * - This example focuses on modem bring-up and AT command interaction only; it does not attach to an APN or open data connections.
 * - All sensitive values (SIM PIN, APN, credentials) should be defined via macros or configuration, not hard-coded for production use.
 * - The CSQ → dBm conversion uses the standard 3GPP TS 27.007 mapping: RSSI_dBm = -113 + 2 × CSQ (for CSQ 0–31, 99 = unknown).
 * - The first character `t` or `T` on a new AT console line is treated as a local command to exit the console and is not sent to the modem.
 *
 * ---------------------------------------------------------------------------
 * 📡 Useful AT Commands You Can Test Manually (In AT console mode)
 * ---------------------------------------------------------------------------
 *
 *  Basic modem response:
 *      AT
 *
 *  Check SIM status:
 *      AT+CPIN?
 *
 *  Check operator / network provider:
 *      AT+COPS?
 *
 *  Scan for available operators (takes time):
 *      AT+COPS=?
 *
 *  Check network registration status:
 *      AT+CREG?
 *
 *  Check GPRS (packet data) attachment:
 *      AT+CGATT?
 *
 *  Check signal strength:
 *      AT+CSQ
 *
 *  Show IMEI:
 *      AT+GSN
 *
 * ---------------------------------------------------------------------------
 *
 * @author Erqos Technologies
 * @date 2025-11-18
 */


#include <EQSP32.h>
#include <TinyGsmClient.h>

// -------------------------------------------------------------
// User configuration
// -------------------------------------------------------------
#define EQX2G_SIM_PIN         ""            // Set SIM PIN "1234" if required, otherwise leave empty

// -------------------------------------------------------------
// Global objects
// -------------------------------------------------------------
EQSP32 eqsp32;
TinyGsm eqx2g(eqx2gStream);   // Modem connected through EQX2G stream

EQTimer disconnectionTimer(2000);   // Check if EQX2G is unplugged

bool modemReady = false;
bool atConsoleMode = false;

// -------------------------------------------------------------
// Forward declarations
// -------------------------------------------------------------
void printMenu();
bool initModem();
void showSignal();
void queryNetworkInfo();
void handleMenuCommands();
void toggleAtConsoleMode();
void handleAtConsole();

// =============================================================
// Setup
// =============================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    eqsp32.begin();  // Standard EQSP32 initialization

    Serial.println("\n📡 Starting EQSP32 EQX2G Demo ...");

    bool isModuleDetected = eqsp32.isModuleDetected(EQX2G());
    Serial.printf("\nThe EQX2G module has%s been detected.\n", isModuleDetected ? "" : " NOT");
    
    if (!isModuleDetected) Serial.printf("Connect an EQX2G module and reboot your EQSP32 .");
    while (!isModuleDetected) {
        Serial.print(".");
        delay(3000);
    }

    if (!initModem()) {
        Serial.println("❌ EQX2G init failed. You can still use 's' to retry.");
    } else {
        Serial.println("✅ EQX2G init OK.");
    }

    printMenu();

    disconnectionTimer.start();
}

// =============================================================
// Main Loop
// =============================================================
void loop() {
    static bool disconnectionDetected = false;

    // Expansion modules are not expected to be unplugged during normal operation.
    // However, we still check periodically so we can detect an unexpected removal
    // and re-initialize the module when it is plugged back in.
    if (disconnectionTimer.isExpired())
    {
        if (!eqsp32.isModuleDetected(EQX2G())) {
            // Module removed
            disconnectionDetected = true;
            Serial.printf("⚠️ EQX2G module has been disconnected.\n");
        } else if (disconnectionDetected) {
            // Module reconnected
            disconnectionDetected = false;

            initModem();        // Reinitialize the EQX2G modem after reconnection
        }
        disconnectionTimer.reset();
    }

    // Handle commands depending on mode
    if (!atConsoleMode) {
        handleMenuCommands();
    } else {
        handleAtConsole();
    }

    delay(10);  // RTOS yield
}

// =============================================================
//  Initialization & Status Helpers
// =============================================================
bool initModem() {
    modemReady = true;

    Serial.println();
    
    Serial.println("[Modem init] Restarting modem ... ");
    eqx2g.restart();

    Serial.print("[Modem init] Pinging modem with AT ... ");
    if (!eqx2g.testAT()) {
        Serial.println("No AT response");
        Serial.println("Aborting ...");
        return false;
    } else {
        Serial.println("AT OK");
    }

    Serial.print("[Modem init] Init SIM with PIN ... ");
    if (!eqx2g.init(EQX2G_SIM_PIN)) {
        Serial.printf("Failed to init using PIN: \"%s\".\n", EQX2G_SIM_PIN);
        modemReady &= false;
    } else {
        Serial.println("Init with PIN OK.");
    }

    // SIM status
    Serial.print("[Modem init] Requesting SIM status ... ");
    int simStatus = eqx2g.getSimStatus();
    Serial.printf("SIM status: %d (%s)\n",
                  simStatus,
                  simStatus == 1 ? "READY" : "NOT READY");

    modemReady &= simStatus == 1;

    return modemReady;
}


// =============================================================
//  Command helpers
// =============================================================
void showSignal() {
    int16_t csq = eqx2g.getSignalQuality();
    int rssiDbm = 0;

    if (csq == 99) {
        Serial.println("[CSQ] Signal: 99 (unknown)");
        return;
    }

    // Convert CSQ → dBm
    rssiDbm = -113 + (csq * 2);

    // Optionally add quality category
    const char* quality;
    if (rssiDbm <= -100) quality = "Very Poor";
    else if (rssiDbm <= -90) quality = "Poor";
    else if (rssiDbm <= -75) quality = "Fair";
    else if (rssiDbm <= -60) quality = "Good";
    else quality = "Excellent";

    Serial.printf("[CSQ] Signal: %d  |  RSSI: %d dBm  |  %s\n",
                  csq, rssiDbm, quality);
}

void queryNetworkInfo() {
    Serial.println("[Network] Checking operator / registration...");

    // TinyGSM has optional helpers depending on SIM800 implementation
    String op = eqx2g.getOperator();
    Serial.printf("  Operator: %s\n", op.c_str());

    int reg = eqx2g.getRegistrationStatus();
    Serial.printf("  Reg Status: %d\n", reg);
}

void printMenu() {
    Serial.println("\n============= EQX2G Command Menu =============");
    Serial.println("  h - show this help");
    Serial.println("  s - re-run full modem health check");
    Serial.println("  c - show current signal quality (CSQ)");
    Serial.println("  n - show network/operator info");
    Serial.println("  t - toggle AT console mode");
    Serial.println("================================================");
}

// =============================================================
// Menu mode → handle one-char commands
// =============================================================
void handleMenuCommands() {
    while (Serial.available() > 0) {
        char cmd = (char)Serial.read();

        switch (cmd) {
            case 'h':
            case 'H':
                printMenu();
                break;

            case 's':
            case 'S':
                initModem();
                break;

            case 'c':
            case 'C':
                showSignal();
                break;

            case 'n':
            case 'N':
                queryNetworkInfo();
                break;

            case 't':
            case 'T':
                toggleAtConsoleMode();
                break;

            case '\r':
            case '\n':
                break;

            default:
                Serial.println("Unknown command. Press 'h' for help.");
                break;
        }
    }
}

// =============================================================
// AT console toggle
// =============================================================
void toggleAtConsoleMode() {
    atConsoleMode = !atConsoleMode;

    if (atConsoleMode) {
        Serial.println("\n[AT Console Mode]");
        Serial.println("Type AT commands and press Enter.");
        Serial.println("Type t or T and press enter to exit to menu.");
    } else {
        Serial.println("\n[Menu Mode]");
        printMenu();
    }
}

// =============================================================
// AT console handler (USB ↔ I2C modem bridge)
// =============================================================
void handleAtConsole() {
    static uint8_t buf[256];
    static size_t n = 0;
    
    bool isCommandTerminated = false;   // Waiting for "\n" to send complete AT command to EQX2G

    // USB → EQX2G
    while (Serial.available() > 0) {
        while (n < sizeof(buf) && Serial.available() > 0) {
            char c = (char)Serial.read();

            if ( (c == 't' || c == 'T') && n == 0) {
                atConsoleMode = false;
                Serial.println("\n[Exited AT Console Mode]");
                printMenu();
                return;
            }

            if (c == '\n')
                isCommandTerminated = true;

            if (c == '\r')
                continue;

            buf[n++] = (uint8_t)c;
        }

        if (n && isCommandTerminated) {
            size_t pushed = eqx2gStream.write(buf, n);
            if (pushed < n) {
                Serial.printf("[WARN] Stream buffer full: wanted %d, sent %d\n", n, pushed);
            }
            n = 0;  // Reset index before leaving
        }
    }

    // EQX2G → USB
    while (eqx2gStream.available() > 0) {
        int b = eqx2gStream.read();
        if (b >= 0) {
            Serial.write((uint8_t)b);
        }
    }
}
