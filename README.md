# RFID-Based Smart Attendance System with Microsoft Excel Logging

[![Arduino](https://img.shields.io/badge/Platform-Arduino%20Uno-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Embedded%20C-00599C.svg?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Bridge-Python%203.x-3776AB.svg?logo=python&logoColor=white)](https://www.python.org/)
[![Excel](https://img.shields.io/badge/Storage-Microsoft%20Excel-217346.svg?logo=microsoftexcel&logoColor=white)](https://www.microsoft.com/excel)
[![Hardware](https://img.shields.io/badge/RFID-MFRC522%20(13.56%20MHz)-blue.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An automated, contactless student attendance recording system powered by an **Arduino Uno R3**, an **MFRC522 13.56 MHz RFID transceiver**, and an **I2C 16x2 character LCD display**. The system captures student card UIDs, cross-references credentials with a pre-configured database, provides immediate multi-sensory feedback (LCD greeting, dual LEDs, buzzer tones), and automatically logs timestamped records directly into a formatted **Microsoft Excel (`.xlsx`)** datasheet.

Developed as a term project for **ECE 202: Electronic Circuit Design Sessional**, Department of Electronics and Communication Engineering, Hajee Mohammad Danesh Science and Technology University (HSTU).

---

## ⚡ Circuit Schematic & Hardware Architecture

<div align="center">
  <img src="docs/assets/circuit_diagram.jpg" alt="RFID Attendance System Circuit Schematic" width="680"/>
  <p><em>Figure 1: Complete circuit schematic and hardware interconnect diagram between Arduino Uno, MFRC522 RFID module, I2C 16x2 LCD, LEDs, and Piezo Buzzer.</em></p>
</div>

---

## 📊 Live Attendance Logging in Microsoft Excel

Every valid swipe is timestamped with system date and time, student name, university registration ID, and status (`Present`), appended automatically in real-time.

<div align="center">
  <img src="docs/assets/excel_output.png" alt="Microsoft Excel Attendance Datasheet Output" width="720"/>
  <p><em>Figure 2: Real-time automated attendance logging output generated in Microsoft Excel.</em></p>
</div>

---

## 🌟 Key Features

- **Contactless 13.56 MHz RFID Identification:** Fast and hygienic badge scanning using the ISO/IEC 14443A standard (MFRC522).
- **Dual-Mode Excel Synchronization:**
  - **Option A (Python Serial Bridge):** An automated Python script (`attendance_logger.py`) that monitors the COM port and writes records into a styled `.xlsx` workbook using `openpyxl`.
  - **Option B (Direct PLX-DAQ):** Direct serial communication protocol enabling real-time macro-driven streaming into open Microsoft Excel sheets.
- **Duplicate Entry Rejection:** Prevents accidental or deliberate double-swiping within the same attendance session.
- **Audio-Visual Real-Time Feedback:**
  - **Authorized:** Green LED illuminates, dual short buzzer beeps, personalized LCD greeting with student ID.
  - **Duplicate:** Green + Red LED blink, warning triple chirp, "Already Marked!" displayed on LCD.
  - **Unauthorized:** Red LED illuminates, long warning beep, "Access Denied!" LCD prompt.
- **Cost-Effective Implementation:** Turnkey hardware constructed using readily available educational electronics components.

---

## 🏗️ Comprehensive System Architecture

The architecture of the RFID Attendance System is structured into **four interconnected operational layers**:

```mermaid
flowchart TB
    subgraph Layer1["1. Physical Transduction Layer"]
        Tag["📇 Passive RFID Tags<br/>(13.56 MHz Mifare Classic 1K)"]
        RC522["📡 MFRC522 Reader Module<br/>(ISO/IEC 14443A SPI @ 3.3V)"]
        Tag -.->|Electromagnetic Induction| RC522
    end

    subgraph Layer2["2. Embedded Logic & Decision Engine"]
        MCU["⚡ Arduino Uno R3 (ATmega328P)<br/>16 MHz Clock | In-Memory Database"]
        AuthLookup{"Credential & Duplicate<br/>Validation Engine"}
        RC522 -->|Hardware SPI Bus<br/>Pins 9, 10, 11, 12, 13| MCU
        MCU --> AuthLookup
    end

    subgraph Layer3["3. Real-Time HMI & Feedback Layer"]
        LCD["📟 16x2 Character LCD<br/>(PCF8574 I2C Backpack - A4/A5)"]
        LED_G["🟢 Green LED (Pin 4)<br/>Access Granted"]
        LED_R["🔴 Red LED (Pin 3)<br/>Access Denied / Duplicate"]
        Buzzer["🔊 Active Piezo Buzzer (Pin 2)<br/>Audio Chimes & Warnings"]
        
        AuthLookup -->|I2C Serial Bus| LCD
        AuthLookup -->|GPIO High/Low| LED_G
        AuthLookup -->|GPIO High/Low| LED_R
        AuthLookup -->|PWM / Digital| Buzzer
    end

    subgraph Layer4["4. Data Synchronization & Persistence Layer"]
        USB["🔌 USB UART Serial Stream<br/>(9600 Baud Protocol)"]
        PythonBridge["🐍 Python Attendance Logger<br/>(pyserial + openpyxl daemon)"]
        PLXDAQ["📊 PLX-DAQ Macro Service<br/>(Direct Active-X Link)"]
        Excel[("📁 Microsoft Excel Datasheet<br/>Attendance_Log.xlsx<br/>[Date, Time, Name, ID, UID]")]
        
        AuthLookup -->|JSON / PLX Stream| USB
        USB --> PythonBridge
        USB --> PLXDAQ
        PythonBridge -->|Append Row| Excel
        PLXDAQ -->|Stream Cells| Excel
    end
```

### Layer Breakdown:
1. **Sensory Identification Layer:** RFID tags enter the 13.56 MHz electromagnetic field generated by the MFRC522 coil antenna, powering the transponder via inductive coupling to transmit its 4-byte/7-byte unique identifier (UID).
2. **Embedded Processing Layer:** The ATmega328P receives the UID over high-speed SPI, validates the credential against the pre-loaded student database, and checks session flags to block duplicate entries.
3. **Local HMI Feedback Layer:** Immediate sensory verification is presented through the 16x2 LCD (personalized name & ID), dual colored LEDs (Green for authenticated, Red for denied), and distinct acoustic buzzer patterns.
4. **Data Synchronization Layer:** Validated attendance transactions are transmitted via USB UART. A lightweight host Python daemon intercepts the stream and safely appends structured rows into a stylized Microsoft Excel datasheet (`.xlsx`).

---

## 🔌 Hardware Pinout Configuration (Aligned with Circuit Schematic)

| Module / Component | Arduino Uno Pin | Protocol / Signal Type | Operating Voltage | Schematic Wire Color |
| :--- | :--- | :--- | :--- | :--- |
| **Active Buzzer** | **Digital Pin 2** | Digital Audio Out | 5.0V | Green wire |
| **Red Status LED** | **Digital Pin 3** | GPIO Out ($220\,\Omega$ $R_1$) | 5.0V | Purple wire |
| **Green Status LED** | **Digital Pin 4** | GPIO Out ($220\,\Omega$ $R_2$) | 5.0V | Cyan / Teal wire |
| **MFRC522 RST** | **Digital Pin 9** | Hardware Reset | 3.3V Logic | Brown wire |
| **MFRC522 SDA (SS)** | **Digital Pin 10** | SPI Slave Select | 3.3V Logic | Purple wire |
| **MFRC522 MOSI** | **Digital Pin 11** | Hardware SPI MOSI | 3.3V Logic | Blue wire |
| **MFRC522 MISO** | **Digital Pin 12** | Hardware SPI MISO | 3.3V Logic | Magenta wire |
| **MFRC522 SCK** | **Digital Pin 13** | Hardware SPI Clock | 3.3V Logic | Orange / Yellow wire |
| **MFRC522 VCC** | **3.3V Header** | Power Supply | ⚠️ **3.3V ONLY** | Red wire |
| **16x2 LCD SDA** | **Analog Pin A4** | I2C Serial Data | 5.0V | Yellow wire |
| **16x2 LCD SCL** | **Analog Pin A5** | I2C Serial Clock | 5.0V | Light Green wire |
| **Common Ground** | **GND Header** | System Ground Rail | 0V | Black common rail |

*For complete electrical guidelines and troubleshooting, see [hardware/circuit_pinout.md](hardware/circuit_pinout.md).*

---

## 🚀 Getting Started

### 1. Arduino Firmware Setup
1. Install the required Arduino libraries via the Library Manager (`Ctrl + Shift + I`):
   - **`MFRC522`** by GithubCommunity
   - **`LiquidCrystal_I2C`** by Frank de Brabander
2. Connect your Arduino Uno board via USB.
3. Open [`firmware/RFID_Attendance_System.ino`](firmware/RFID_Attendance_System.ino) in the Arduino IDE.
4. Select `Tools > Board > Arduino Uno` and choose your port.
5. Click **Upload** (`Ctrl + U`).

### 2. Python-to-Excel Bridge Setup (Recommended)
To automatically record swipe events directly into `Attendance_Log.xlsx`:

```bash
# Navigate to the python bridge directory
cd python_bridge

# Install Python dependencies
pip install -r requirements.txt

# Run the attendance daemon
python attendance_logger.py
```
The script will automatically detect your Arduino's serial port and create/update the formatted Excel file in real time.

---

## 📂 Repository Structure

```
rfid-attendance-system-arduino-excel/
├── firmware/
│   ├── RFID_Attendance_System.ino   # Main embedded sketch with MFRC522 & I2C LCD
│   └── config.h                      # Pin definitions, timing, & registered student database
├── python_bridge/
│   ├── attendance_logger.py          # Python daemon logging serial telemetry to Excel
│   └── requirements.txt              # Python dependencies (pyserial, openpyxl, pandas)
├── hardware/
│   └── circuit_pinout.md             # Detailed wiring diagram, pin mapping, and voltage specs
├── docs/
│   └── assets/                       # Circuit schematic, flowchart, and Excel output screenshots
├── .gitignore                        # Clean ignore rules for builds, temporary Excel locks
├── LICENSE                           # MIT Open-Source License
└── README.md                         # Project documentation
```

---

## 👥 Authors & Project Team

**Group 08 — Level 2, Semester I**  
Department of Electronics and Communication Engineering  
Hajee Mohammad Danesh Science and Technology University (HSTU), Dinajpur, Bangladesh

- **Md. Al-Imran** (Student ID: 2102129) — *Lead Developer & Embedded Firmware*  
  GitHub: [@imran7976](https://github.com/imran7976) | Email: ai.skhimran01@gmail.com
- **Mukund Mohan Mishra** (Student ID: 2102161)
- **Md. Mohsin Ahmed** (Student ID: 2102108)
- **Md. Abul Kashem** (Student ID: 2102115)

**Course Instructor / Supervisor:**  
**Md. Abubakar Siddik**, *Assistant Professor*, Dept. of ECE, HSTU
