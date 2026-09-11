/**
 * @file EQSP32_EQXHI_Demo.ino
 * @brief Demonstrates the EQXHI isolated AC/DC voltage-presence input expansion module.
 *
 * The EQXHI provides 10 isolated inputs that report whether field voltage is present,
 * plus a per-input frequency quality/error byte. It is an input-only module: it has no
 * outputs and no configurable pin modes.
 *
 * Channel Map:
 * - Channels 1-10  (`EQXHI_IN_1` .. `EQXHI_IN_10`): debounced voltage-presence state.
 *   Read with `readPin()`; positive logic, 1 means field voltage present.
 * - Channels 11-20 (`EQXHI_QUALITY_1` .. `EQXHI_QUALITY_10`): frequency quality/error byte
 *   for the matching input. Decode it with `EQXHI_GET_FREQ_QUALITY()`, which returns an
 *   `EQXHI_FREQ_QUALITY_*` code, and `EQXHI_HAS_DELAYED_FREQ_ERROR()`, which reports the
 *   delayed frequency-deviation error.
 *
 * Frequency Monitoring:
 * - `configFREQ()` sets the expected frequency, the allowed deviation, and how long a
 *   deviation must persist before the channel reports a delayed frequency error.
 * - Configuration is shared per common group: group 1 covers IN1-IN5 and group 2 covers
 *   IN6-IN10. Any input pin of a group configures that whole group; `EQXHI_COM_GROUP_1`
 *   and `EQXHI_COM_GROUP_2` are the representative pins that make the intent explicit.
 * - A target of 0 selects DC expectation, otherwise the target is 25-1000 Hz.
 * - A deviation of 0 disables monitoring for the whole group and overrides the target, so
 *   the group would never report a frequency error. Keep it non-zero to arm monitoring.
 * - By default `DEMO_GROUP_2_DC` is 1, so IN6-IN10 are watched as a DC supply. Set it to 0
 *   to expect 60 Hz mains on that group instead.
 *   With monitoring armed, a qualified DC input grades GOOD and a periodic (AC) input grades
 *   BAD; an absent or still-acquiring input reads N/A or ACQUIRING rather than BAD.
 * - The configuration is RAM-only on the module and returns to defaults after a module
 *   reset, so this demo reapplies it whenever the module reconnects.
 *
 * Input-Only Behavior:
 * - `pinValue()` is rejected and returns false.
 * - `pinMode()` accepts DIN as a no-op and rejects every other mode.
 * - `readMode()` returns DIN for the physical input channels 1-10. Channels 11-20 are
 *   virtual channels that exist only to address a value, so they have no pin mode.
 *
 * Hardware Setup:
 * - EQXHI module connected to the EQSP32 expansion port, indexed as 1.
 * - AC or DC field voltage wired to the EQXHI inputs.
 * - IN1 is used for the edge-trigger example; it belongs to common group 1.
 *
 * Detection Behavior:
 * - EQXHI modules are auto-detected during the initial eqsp32.begin() call.
 * - If a module is not connected at boot time, it will not be recognized later.
 *
 * Features:
 * - Reads voltage-presence state with STATE / ON_RISING / ON_FALLING / ON_TOGGLE triggers.
 * - Decodes and prints the frequency quality code and the delayed frequency error flag.
 * - Configures 50 Hz mains monitoring on group 1 and DC monitoring on group 2, with a
 *   compile-time switch to expect 60 Hz mains on group 2 instead.
 * - Handles module disconnect and reconnect, reapplying the RAM-only configuration.
 *
 * @author Erqos Technologies
 * @date 2026-09-09
 */

#include <EQSP32.h>

EQSP32 eqsp32;

// -------------------- Configuration --------------------
// Define EQXHI module index (per module type, 1-based)
#define EQXHI_INDEX             1

#define EQXHI_MODULE            EQXHI(EQXHI_INDEX)

// IN1 drives the edge-trigger example below
#define EQXHI_TRIGGER_PIN       EQXHI(EQXHI_INDEX, EQXHI_IN_1)

// Trigger mode for the voltage-presence read
#define INPUT_TRIGGER_MODE      ON_RISING   // try: STATE / ON_RISING / ON_FALLING / ON_TOGGLE

// Demo selection switch: what group 2 (IN6-IN10) is expected to carry
#define DEMO_GROUP_2_DC         1           // 0 = 60 Hz AC mains, 1 = DC

// Frequency-monitor profile, applied per common group
#define GROUP_1_TARGET_HZ       50          // IN1-IN5 expect 50 Hz mains
#if DEMO_GROUP_2_DC
    #define GROUP_2_TARGET_HZ   0           // IN6-IN10 expect DC; a target of 0 selects DC expectation
#else
    #define GROUP_2_TARGET_HZ   60          // IN6-IN10 expect 60 Hz mains
#endif

// Allowed drift from the target, in whole Hz. Keep this non-zero: a deviation of 0 disables
// monitoring for the whole group and overrides the target, so the group would never report a
// frequency error.
#define FREQ_DEVIATION_HZ       5
#if DEMO_GROUP_2_DC
    // On a DC group the deviation is only an on/off switch: any nonzero value arms monitoring
    // and the number itself is not used, since a DC input is graded on being DC, not on drift.
    #define GROUP_2_DEVIATION_HZ 1
#else
    #define GROUP_2_DEVIATION_HZ FREQ_DEVIATION_HZ
#endif
#define FREQ_TRIGGER_DELAY_MS   500         // Deviation must persist this long before it is reported

// Demo timing
#define LOOP_DELAY_MS           100
#define SCAN_EVERY_N_LOOPS      20          // Full channel scan every 2 s at 100 ms per loop

// -------------------- Helpers --------------------
bool configureEQXHI();
void printChannelScan();
void printQuality(int qualityByte);
void demoInputOnlyBehavior();

// -------------------- Setup --------------------
void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println("\n🚀 Starting EQSP32 EQXHI Demo...");

    eqsp32.begin();

    if (!eqsp32.isModuleDetected(EQXHI_MODULE)) {
        Serial.printf("❌ EQXHI module %d not detected. Connect the module and reboot.\n", EQXHI_INDEX);
        return;
    }
    Serial.printf("✅ EQXHI module %d detected.\n", EQXHI_INDEX);

    configureEQXHI();
    demoInputOnlyBehavior();
}

// -------------------- Loop --------------------
void loop() {
    static bool disconnectionDetected = false;
    static uint32_t loopCount = 0;

    // -------------------- Handle disconnect / reconnect --------------------
    if (!eqsp32.isModuleDetected(EQXHI_MODULE)) {
        if (!disconnectionDetected) {
            disconnectionDetected = true;
            Serial.println("⚠️ EQXHI module disconnected!");
        }
        delay(1000);
        return;
    } else if (disconnectionDetected) {
        disconnectionDetected = false;
        Serial.println("✅ EQXHI module reconnected. Reapplying frequency configuration...");

        // Module configuration is RAM-only, so it is lost on a module reset and must be reapplied.
        configureEQXHI();
    }

    // -------------------- Edge-triggered voltage presence --------------------
    if (eqsp32.readPin(EQXHI_TRIGGER_PIN, INPUT_TRIGGER_MODE) > 0) {
        Serial.print("\n⚡ IN1 trigger fired (mode = ");
        switch (INPUT_TRIGGER_MODE) {
            case STATE:      Serial.print("STATE");      break;
            case ON_RISING:  Serial.print("ON_RISING");  break;
            case ON_FALLING: Serial.print("ON_FALLING"); break;
            case ON_TOGGLE:  Serial.print("ON_TOGGLE");  break;
            default:         Serial.print("UNKNOWN");    break;
        }
        Serial.println(")");
    }

    // -------------------- Periodic full channel scan --------------------
    if ((loopCount % SCAN_EVERY_N_LOOPS) == 0) {
        printChannelScan();
    }
    loopCount++;

    delay(LOOP_DELAY_MS);
}

// -------------------- EQXHI Configuration --------------------
/**
 * @brief Applies the frequency-monitor profile to both EQXHI common groups.
 *
 * Any input pin of a group configures that entire group, so a single call per group is
 * enough. `configFREQ()` writes each value separately, so a partial failure returns false
 * while the accepted values stay applied; retry on failure.
 *
 * @return true only if both groups were fully configured.
 */
bool configureEQXHI() {
    Serial.println("\nConfiguring EQXHI frequency monitoring...");

    bool group1 = eqsp32.configFREQ(EQXHI(EQXHI_INDEX, EQXHI_COM_GROUP_1),
                                    GROUP_1_TARGET_HZ, FREQ_DEVIATION_HZ, FREQ_TRIGGER_DELAY_MS);
    bool group2 = eqsp32.configFREQ(EQXHI(EQXHI_INDEX, EQXHI_COM_GROUP_2),
                                    GROUP_2_TARGET_HZ, GROUP_2_DEVIATION_HZ, FREQ_TRIGGER_DELAY_MS);

    // Any pin of a group is equivalent, this would configure group 2 exactly as the call above:
    // eqsp32.configFREQ(EQXHI(EQXHI_INDEX, EQXHI_IN_8), GROUP_2_TARGET_HZ, GROUP_2_DEVIATION_HZ, FREQ_TRIGGER_DELAY_MS);

    // Disable frequency monitoring on a group:
    // eqsp32.configFREQ(EQXHI(EQXHI_INDEX, EQXHI_COM_GROUP_1), 0, 0, 0);

    Serial.printf(" - Group 1 (IN1-IN5)  @ %d Hz ±%d Hz / %d ms: %s\n",
                  GROUP_1_TARGET_HZ, FREQ_DEVIATION_HZ, FREQ_TRIGGER_DELAY_MS,
                  group1 ? "OK" : "FAILED (retry)");
#if DEMO_GROUP_2_DC
    // A DC group is graded on whether the input is DC, so the deviation band is not part of it.
    Serial.printf(" - Group 2 (IN6-IN10) @ DC / %d ms: %s\n",
                  FREQ_TRIGGER_DELAY_MS,
                  group2 ? "OK" : "FAILED (retry)");
#else
    Serial.printf(" - Group 2 (IN6-IN10) @ %d Hz ±%d Hz / %d ms: %s\n",
                  GROUP_2_TARGET_HZ, GROUP_2_DEVIATION_HZ, FREQ_TRIGGER_DELAY_MS,
                  group2 ? "OK" : "FAILED (retry)");
#endif

    return group1 && group2;
}

// -------------------- Input-Only Behavior --------------------
/**
 * @brief Shows how the EQXHI reports itself as an input-only module.
 */
void demoInputOnlyBehavior() {
    Serial.println("\nEQXHI is input-only:");

    // Writes are always rejected.
    bool written = eqsp32.pinValue(EQXHI_TRIGGER_PIN, 1000);
    Serial.printf(" - pinValue() on IN1        : %s\n", written ? "accepted" : "rejected (expected)");

    // DIN is accepted as a no-op, every other mode is rejected.
    bool dinAccepted = eqsp32.pinMode(EQXHI_TRIGGER_PIN, DIN);
    bool poutAccepted = eqsp32.pinMode(EQXHI_TRIGGER_PIN, POUT);
    Serial.printf(" - pinMode(DIN)  on IN1     : %s\n", dinAccepted ? "accepted (expected)" : "rejected");
    Serial.printf(" - pinMode(POUT) on IN1     : %s\n", poutAccepted ? "accepted" : "rejected (expected)");

    // Physical input channels always report DIN.
    EQ_PinMode inputMode = eqsp32.readMode(EQXHI_TRIGGER_PIN);
    Serial.printf(" - readMode() on IN1        : %d (%s)\n", inputMode, inputMode == DIN ? "DIN" : "NOT DIN");
}

// -------------------- Channel Scan --------------------
/**
 * @brief Prints the voltage-presence state and frequency quality of all 10 inputs.
 */
void printChannelScan() {
    Serial.println("\n========================================");
    Serial.println("📡 EQXHI channel scan\n");

    for (int ch = 1; ch <= EQXHI_NUM_INPUTS; ch++) {
        // Channels 1-10 hold the debounced voltage-presence state
        int state = eqsp32.readPin(EQXHI(EQXHI_INDEX, ch));

        // Channels 11-20 hold the matching frequency quality/error byte
        int quality = eqsp32.readPin(EQXHI(EQXHI_INDEX, EQXHI_QUALITY_1 + (ch - 1)));

        // readPin() returns -1 if a channel is unavailable, so test for 1 rather than
        // truthiness; otherwise an error would be reported as voltage present.
        const char *stateText = (state == 1) ? "🔴 VOLTAGE " : (state == 0) ? "⚫ NONE    " : "❓ ERROR   ";

        Serial.printf("IN%-2d  %s  ", ch, stateText);
        printQuality(quality);
        Serial.println();
    }
}

/**
 * @brief Decodes an EQXHI frequency quality/error byte.
 *
 * The byte packs two things: EQXHI_GET_FREQ_QUALITY() returns the live quality code and
 * EQXHI_HAS_DELAYED_FREQ_ERROR() reports the delayed frequency-deviation error, which is
 * set once a deviation has persisted for the configured trigger delay.
 *
 * @param qualityByte Value returned by a quality channel read.
 */
void printQuality(int qualityByte) {
    uint8_t quality = EQXHI_GET_FREQ_QUALITY(qualityByte);

    switch (quality) {
        case EQXHI_FREQ_QUALITY_NOT_AVAILABLE: Serial.print("quality: N/A          "); break;
        case EQXHI_FREQ_QUALITY_ACQUIRING:     Serial.print("quality: ACQUIRING    "); break;
        case EQXHI_FREQ_QUALITY_BAD:           Serial.print("quality: BAD          "); break;
        case EQXHI_FREQ_QUALITY_MARGINAL:      Serial.print("quality: MARGINAL     "); break;
        case EQXHI_FREQ_QUALITY_GOOD:          Serial.print("quality: GOOD         "); break;
        case EQXHI_FREQ_QUALITY_EXCELLENT:     Serial.print("quality: EXCELLENT    "); break;
        case EQXHI_FREQ_QUALITY_NOT_MONITORED: Serial.print("quality: NOT_MONITORED"); break;
        default:                               Serial.print("quality: UNKNOWN      "); break;
    }

    if (EQXHI_HAS_DELAYED_FREQ_ERROR(qualityByte)) {
        Serial.print("  ⚠️ FREQ DEVIATION ERROR");
    }
}
