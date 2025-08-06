/**
 * @file EQSP32_SMTP_Email_Alert_Demo.ino
 * @brief Demonstrates sending an email using Gmail's SMTP server with EQSP32 and ESP-Mail-Client.
 *
 * This example shows how to send an automatic email notification (e.g., after power reset)
 * using the EQSP32 device connected via Wi-Fi.
 *
 * Features:
 * - Connects to Gmail's SMTP server (smtp.gmail.com).
 * - Sends a simple email to a specified recipient.
 * - Uses App Password (required for Gmail with 2FA).
 * - Displays SMTP status and response details.
 *
 * Requirements:
 * - Active internet connection specifically via Wi-Fi.
 *   The SMTP requires a secure connection with the host, which is not yet supported while using Ethernet.
 * - ESP-Mail-Client library installed.
 * - App Password from Gmail if 2-Factor Authentication is enabled.
 *
 * Setup Instructions:
 * - Replace AUTHOR_EMAIL, AUTHOR_PASSWORD, and RECIPIENT_EMAIL with your own.
 * - Use port 465 for SSL or 587 for STARTTLS.
 * - Provision EQSP32 Wi-Fi using EQConnect.
 *
 * Reference:
 * https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/
 *
 * @author Erqos Technologies
 * @date 2025-08-05
 */

#include "EQSP32.h"
#include "ESP_Mail_Client.h"

// Email credentials - Replace these with your own details
#define SMTP_HOST "smtp.gmail.com"  // Gmail SMTP server
#define SMTP_PORT 465               // SSL port (use 587 for STARTTLS)


// ====== 📧 Replace these placeholders with your real email credentials ======
#define AUTHOR_EMAIL      "your_email@gmail.com"   // Sender Gmail address
#define AUTHOR_PASSWORD   "your_app_password"      // App Password (not your Gmail password!)
#define RECIPIENT_EMAIL   "recipient@example.com"  // Recipient email address
// =============================================================================


// EQSP32 setup for Wi-Fi provisioning
EQSP32 eqsp32;

// ESP Mail Client session for sending email
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Local time:)");
      eqsp32.printLocalTime();
      Serial.printf("Recipient: %s\n", result.recipients.c_str());
      Serial.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

void setup() {
    Serial.begin(115200);

    // Start EQSP32 with verbose mode disabled
    eqsp32.begin(); // true enables verbose mode

    // Wait for EQSP32 to connect to Wi-Fi
    Serial.print("\n📶 Connecting to Wi-Fi ");
    while (eqsp32.getWiFiStatus() != EQ_WF_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" ✅");
    Serial.println("🌐 Device is online.");

    // Wait for local time sync via NTP
    Serial.print("⏳ Syncing local time ");
    while (!eqsp32.isLocalTimeSynced()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" ✅");
    Serial.println("🕒 Local time is synced.");

    // Configure SMTP session
    Session_Config config;
    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = ""; // Leave empty unless using domain-based SMTP

    // Enable debugging for SMTP communication
    smtp.debug(1);

    // Set the callback function for SMTP status updates
    smtp.callback(smtpCallback);

    // Connect to the SMTP server
    if (!smtp.connect(&config)) {
        Serial.println("Failed to connect to SMTP server.");
        return;
    }

    // Compose the email message
    SMTP_Message message;
    message.sender.name = eqsp32.getDeviceName();  // Sender's name
    message.sender.email = AUTHOR_EMAIL;  // Sender's email
    message.subject = "EQSP32 Power Reset Notification";  // Email subject
    message.addRecipient("User", RECIPIENT_EMAIL);  // Recipient's name and email

    // Email body content (plain text)
    String emailMessage = "The EQSP32 device has just restarted.\nIt is now online and time-synced as of ";
    emailMessage += String(eqsp32.getFormattedLocalTime().c_str());
    emailMessage += " (local time).";
    message.text.content = emailMessage.c_str();

    // Send the email
    if (MailClient.sendMail(&smtp, &message, true)) {
        Serial.println("Email sent successfully.");
    } else {
        Serial.println("Failed to send email: " + smtp.errorReason());
    }
}

void loop() {
  // Idle loop to keep the program running
  delay(10000);  // Wait for 10 seconds before repeating the loop
}
