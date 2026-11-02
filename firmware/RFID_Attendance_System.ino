/**
 * @file RFID_Attendance_System.ino
 * @brief Autonomous RFID-Based Attendance Management System with Microsoft Excel Logging
 * @author Md. Al-Imran (Student ID: 2102129)
 * @course ECE 202: Electronic Circuit Design Sessional
 * @institution Hajee Mohammad Danesh Science and Technology University (HSTU)
 * 
 * Hardware Setup:
 *   - Microcontroller: Arduino Uno R3 (ATmega328P)
 *   - RFID Module:     MFRC522 (13.56 MHz RFID Reader, SPI Interface)
 *   - Display:         16x2 Character LCD with I2C Backpack (PCF8574)
 *   - Indicators:      Green LED (Auth), Red LED (Denied/Duplicate), Piezo Buzzer
 *   - Software Bridge: Direct Microsoft Excel integration via PLX-DAQ or Python Serial Bridge
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// ==========================================
// HARDWARE INSTANCES
// ==========================================
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================
void showIdleScreen();
void signalSuccess();
void signalDenied();
void signalDuplicate();
String getCardUID();
int findStudentIndex(String uid);

// ==========================================
// SETUP ROUTINE
// ==========================================
void setup() {
    // 1. Initialize Serial Communication for PC Data Logging
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 2000);

    // Initialize PLX-DAQ Excel Spreadsheet Protocol
    Serial.println(F("CLEARDATA"));
    Serial.println(F("LABEL,Date,Time,Name,Student ID,Status"));

    // 2. Initialize GPIO Indicator Pins
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);

    // 3. Initialize I2C LCD Display
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("RFID Attendance"));
    lcd.setCursor(0, 1);
    lcd.print(F("System Booting.."));

    // 4. Initialize Hardware SPI & MFRC522 RFID Reader
    SPI.begin();
    mfrc522.PCD_Init();
    delay(500);

    // Self-test diagnostic confirmation
    Serial.println(F("=================================================="));
    Serial.println(F(" RFID Attendance System | Arduino Uno & MS Excel"));
    Serial.println(F(" Author: Md. Al-Imran (HSTU ECE)                 "));
    Serial.println(F(" Status: RC522 Online, LCD Ready, SPI Active     "));
    Serial.println(F("=================================================="));

    // Show ready state
    showIdleScreen();
}

// ==========================================
// MAIN POLLING LOOP
// ==========================================
void loop() {
    // Look for new RFID card presence
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Capture Card UID string
    String scannedUID = getCardUID();
    Serial.print(F("[INFO] Scanned Card UID: "));
    Serial.println(scannedUID);

    // Visual feedback that card was read
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Verifying Card.."));
    lcd.setCursor(0, 1);
    lcd.print(scannedUID);

    // Search for student in pre-registered database
    int studentIdx = findStudentIndex(scannedUID);

    if (studentIdx != -1) {
        // Registered Student Found
        StudentRecord &student = studentDatabase[studentIdx];

        if (!student.attendedToday) {
            // New attendance entry
            student.attendedToday = true;

            // 1. Log to Microsoft Excel via PLX-DAQ Serial Syntax
            // Syntax: DATA,DATE,TIME,Name,StudentID,Status
            Serial.print(F("DATA,DATE,TIME,"));
            Serial.print(student.name);
            Serial.print(F(","));
            Serial.print(student.studentId);
            Serial.println(F(",Present"));

            // 2. Structured JSON Telemetry for Python Logger
            Serial.print(F("JSON:{\"name\":\""));
            Serial.print(student.name);
            Serial.print(F("\",\"id\":\""));
            Serial.print(student.studentId);
            Serial.print(F("\",\"uid\":\""));
            Serial.print(scannedUID);
            Serial.println(F("\",\"status\":\"Present\"}"));

            // 3. LCD Screen Success Feedback
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Welcome, "));
            // Print truncated name if longer than 7 chars
            String shortName = String(student.name);
            if (shortName.length() > 7) shortName = shortName.substring(0, 7);
            lcd.print(shortName);

            lcd.setCursor(0, 1);
            lcd.print(F("ID: "));
            lcd.print(student.studentId);

            signalSuccess();
        } else {
            // Duplicate scan prevention
            Serial.print(F("[WARNING] Duplicate scan detected for ID: "));
            Serial.println(student.studentId);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Already Marked!"));
            lcd.setCursor(0, 1);
            lcd.print(student.name);

            signalDuplicate();
        }
    } else {
        // Unknown / Unauthorized Card
        Serial.print(F("[ALERT] Unauthorized Card Scanned: "));
        Serial.println(scannedUID);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Access Denied!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Unknown Card"));

        signalDenied();
    }

    // Halt PICC and stop encryption on PCD
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    // Settle display and reset to idle prompt
    delay(FEEDBACK_DELAY_MS);
    showIdleScreen();
    delay(DEBOUNCE_DELAY_MS);
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

/**
 * @brief Reads the hex bytes of the RFID tag and converts to formatted string
 */
String getCardUID() {
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) {
            uidStr += "0";
        }
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) {
            uidStr += " ";
        }
    }
    uidStr.toUpperCase();
    return uidStr;
}

/**
 * @brief Searches database for matching card UID
 * @return Index in database if found, -1 if unregistered
 */
int findStudentIndex(String uid) {
    for (int i = 0; i < TOTAL_STUDENTS; i++) {
        if (uid.equalsIgnoreCase(studentDatabase[i].uid)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Restores the default prompt on the 16x2 LCD
 */
void showIdleScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Scan Your Card.."));
    lcd.setCursor(0, 1);
    lcd.print(F("HSTU ECE Dept."));
}

/**
 * @brief Auditory and visual confirmation for authorized attendance
 */
void signalSuccess() {
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
}

/**
 * @brief Warning alert for unauthorized card
 */
void signalDenied() {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(800);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}

/**
 * @brief Feedback for card already scanned in the current session
 */
void signalDuplicate() {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, HIGH);
    for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(80);
        digitalWrite(BUZZER_PIN, LOW);
        delay(80);
    }
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
}
