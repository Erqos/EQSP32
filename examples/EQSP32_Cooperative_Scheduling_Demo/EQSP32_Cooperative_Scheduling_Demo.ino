/**
 * @file EQSP32_Cooperative_Scheduling_Demo.ino
 * @brief Demonstrates using a cooperative task scheduler (TaskScheduler library) 
 *        within the EQSP32 framework. This example schedules two non-blocking tasks 
 *        using fixed intervals to read a digital input and control a PWM output.
 * 
 * Key Concepts:
 * - This example uses the TaskScheduler library to manage periodic tasks inside the main loop.
 * - The EQSP32 system runs its background tasks using FreeRTOS at higher priority than loop() by design, 
 *   to avoid stalling due to loop() blocking operations.
 * 
 * What to Pay Attention To:
 * - Cooperative tasks MUST NOT block. Avoid long loops, delays > 10ms, or blocking IO.
 * - Use short task bodies that return quickly.
 * - EQSP32 system functions (e.g., reading pins) are safe to use since they are non-blocking.
 * - Always include delay(10) or similar at the end of loop() to allow FreeRTOS tasks to execute properly.
 * 
 * Requirements:
 * - Install the TaskScheduler library by Arkhipenko from Library Manager.
 * - Connect a digital input device (e.g., button) to PIN_INPUT and a PWM output device (e.g., LED) to PIN_OUTPUT.
 * 
 * @author Erqos Technologies
 * @date 2025-06-11
 */

#include <EQSP32.h>
#include <TaskScheduler.h>

#define PIN_INPUT   3   // Digital input pin (e.g., button)
#define PIN_OUTPUT  9   // PWM output pin (e.g., LED or LED strip)

EQSP32 eqsp32;
Scheduler userScheduler;  // TaskScheduler instance

// State variable
bool isButtonPressed = false;

// Tasks
void readInputTaskCallback();
void updatePWMTaskCallback();

Task readInputTask(500, TASK_FOREVER, &readInputTaskCallback, &userScheduler);    // Every 500 ms
Task updatePWMTask(1000, TASK_FOREVER, &updatePWMTaskCallback, &userScheduler);   // Every 1 s

void setup() {
    Serial.begin(115200);
    eqsp32.begin();  // Starts EQSP32 supervisor tasks

    // Configure pins
    eqsp32.pinMode(PIN_INPUT, SWT);     // Button input with debounce
    eqsp32.pinMode(PIN_OUTPUT, POUT);   // 24V LED output
    eqsp32.pinValue(PIN_OUTPUT, 0);     // Initial OFF

    // Start scheduler tasks
    readInputTask.enable();
    updatePWMTask.enable();

    Serial.println("📋 Cooperative scheduler demo started.");
}

void loop() {
    userScheduler.execute();  // Run all enabled cooperative tasks
    
    delay(10);  // Always allow time for background tasks
}

// --- Task Callbacks ---

void readInputTaskCallback() {
    isButtonPressed = eqsp32.readPin(PIN_INPUT);  // Read switch input
    Serial.printf("🔘 Button state: %s\n", isButtonPressed ? "HIGH" : "LOW");
}

void updatePWMTaskCallback() {
    static int duty = 0;

    if (isButtonPressed) {
        duty += 250;
        if (duty > 1000) duty = 0;

        eqsp32.pinValue(PIN_OUTPUT, duty);
        Serial.printf("💡 PWM output (pin %d): %d/1000\n", PIN_OUTPUT, duty);
    } else {
        eqsp32.pinValue(PIN_OUTPUT, 0);  // Force LED OFF
        Serial.printf("💡 PWM output (pin %d): OFF (button not pressed)\n", PIN_OUTPUT);
    }
}
