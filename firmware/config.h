/**
 * @file config.h
 * @brief Configuration and Student Database for RFID-Based Attendance System
 * @author Md. Al-Imran (Student ID: 2102129)
 * @institution Hajee Mohammad Danesh Science and Technology University (HSTU)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. HARDWARE PIN DEFINITIONS
// ==========================================
// RC522 SPI Pinout
#define SS_PIN          10  // SPI Slave Select (SDA)
#define RST_PIN         9   // Reset pin
// MOSI = Pin 11, MISO = Pin 12, SCK = Pin 13 (Hardware SPI)

// Indicators & Alerts (Matching Schematic)
#define BUZZER_PIN      2   // Green wire: Piezo Buzzer
#define LED_RED_PIN     3   // Purple wire: Red LED (220 ohm resistor)
#define LED_GREEN_PIN   4   // Cyan/Teal wire: Green LED (220 ohm resistor)

// I2C 16x2 LCD Display Configuration
#define LCD_I2C_ADDR    0x27  // Standard I2C address (use 0x3F if 0x27 doesn't display)
#define LCD_COLS        16
#define LCD_ROWS        2

// ==========================================
// 2. TIMING & SYSTEM CONSTANTS
// ==========================================
#define SERIAL_BAUD_RATE    9600
#define FEEDBACK_DELAY_MS   2000  // Time to show user message before clearing display
#define DEBOUNCE_DELAY_MS   1500  // Minimum delay between consecutive scans

// ==========================================
// 3. STUDENT DATABASE STRUCTURE
// ==========================================
struct StudentRecord {
    const char *uid;        // RFID Card UID in uppercase hex (e.g. "E5 8D 08 88")
    const char *name;       // Student Full Name
    const char *studentId;  // University Student ID Number
    bool attendedToday;     // Daily attendance flag (prevents duplicates)
};

// Database populated with Group 08 student records from the academic report
static StudentRecord studentDatabase[] = {
    { "43 78 8B 0D", "IMRAN",               "2102129", false },
    { "69 8D 08 88", "MUKUND MOHAN MISHRA", "2102161", false },
    { "52 4F A1 2C", "MD. MOHSIN AHMED",    "2102108", false },
    { "B4 3E C9 15", "MD. ABUL KASHEM",     "2102115", false }
};

const int TOTAL_STUDENTS = sizeof(studentDatabase) / sizeof(studentDatabase[0]);

#endif // CONFIG_H
