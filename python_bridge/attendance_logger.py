"""
RFID Attendance System - Python to Excel Serial Bridge
Author: Md. Al-Imran (HSTU ECE)

Reads serial telemetry from Arduino Uno over USB COM port,
extracts real-time timestamped student records, and appends them
into a styled Microsoft Excel (.xlsx) workbook using openpyxl.
"""

import sys
import time
import json
from datetime import datetime
import os

try:
    import serial
    import serial.tools.list_ports
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    print("Missing required libraries. Please run:")
    print("pip install pyserial openpyxl")
    sys.exit(1)

EXCEL_FILE = "Attendance_Log.xlsx"
DEFAULT_BAUD = 9600

def find_arduino_port():
    """Automatically detects connected Arduino Uno COM port."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        desc = (port.description or "").lower()
        hwid = (port.hwid or "").lower()
        if "arduino" in desc or "ch340" in desc or "usb-serial" in desc or "2341" in hwid:
            return port.device
    if ports:
        return ports[0].device
    return None

def setup_excel_file():
    """Initializes the Excel spreadsheet with professional styling and column headers."""
    if not os.path.exists(EXCEL_FILE):
        wb = Workbook()
        ws = wb.active
        ws.title = "Attendance Records"

        headers = ["Date", "Time", "Student Name", "Student ID", "RFID UID", "Status"]
        ws.append(headers)

        header_font = Font(name="Calibri", size=11, bold=True, color="FFFFFF")
        header_fill = PatternFill(start_color="1F497D", end_color="1F497D", fill_type="solid")
        alignment = Alignment(horizontal="center", vertical="center")
        thin_border = Border(
            left=Side(style='thin', color='D9D9D9'),
            right=Side(style='thin', color='D9D9D9'),
            top=Side(style='thin', color='D9D9D9'),
            bottom=Side(style='thin', color='D9D9D9')
        )

        for col_num in range(1, len(headers) + 1):
            cell = ws.cell(row=1, column=col_num)
            cell.font = header_font
            cell.fill = header_fill
            cell.alignment = alignment

        column_widths = [14, 14, 26, 16, 18, 14]
        for i, width in enumerate(column_widths, 1):
            col_letter = chr(64 + i)
            ws.column_dimensions[col_letter].width = width

        wb.save(EXCEL_FILE)
        print(f"[INIT] Created new attendance datasheet: {EXCEL_FILE}")

def log_attendance(name, student_id, uid, status="Present"):
    """Appends a new student check-in row to the Excel sheet."""
    now = datetime.now()
    date_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%I:%M:%S %p")

    wb = load_workbook(EXCEL_FILE)
    ws = wb["Attendance Records"]

    row_data = [date_str, time_str, name, student_id, uid, status]
    ws.append(row_data)

    current_row = ws.max_row
    align_center = Alignment(horizontal="center", vertical="center")
    align_left = Alignment(horizontal="left", vertical="center")

    for col in range(1, 7):
        cell = ws.cell(row=current_row, column=col)
        cell.alignment = align_left if col == 3 else align_center

    wb.save(EXCEL_FILE)
    print(f"[{time_str}] Successfully Logged: {name} (ID: {student_id}) -> {EXCEL_FILE}")

def main():
    print("==================================================")
    print(" RFID Attendance System - Python-Excel Data Bridge")
    print(" Author: Md. Al-Imran (HSTU ECE)                  ")
    print("==================================================")

    setup_excel_file()

    port = find_arduino_port()
    if not port:
        print("[ERROR] No Arduino found! Check USB connection.")
        port = input("Enter COM port manually (e.g. COM3): ").strip()

    print(f"[CONNECTING] Opening serial connection on {port} @ {DEFAULT_BAUD} baud...")
    try:
        ser = serial.Serial(port, DEFAULT_BAUD, timeout=1)
        time.sleep(2)  # Wait for Arduino bootloader reset
        print(f"[ONLINE] Connected to {port}. Ready to record attendance!\n")
    except Exception as e:
        print(f"[FATAL] Failed to connect on {port}: {e}")
        return

    try:
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                # Parse JSON stream from Arduino
                if line.startswith("JSON:"):
                    json_str = line[5:]
                    try:
                        data = json.loads(json_str)
                        log_attendance(
                            name=data.get("name", "Unknown"),
                            student_id=data.get("id", "N/A"),
                            uid=data.get("uid", "N/A"),
                            status=data.get("status", "Present")
                        )
                    except json.JSONDecodeError:
                        pass
                elif line.startswith("DATA,"):
                    # Fallback parse for standard PLX-DAQ format
                    parts = line.split(",")
                    if len(parts) >= 6:
                        log_attendance(
                            name=parts[3],
                            student_id=parts[4],
                            uid="N/A",
                            status=parts[5]
                        )
                else:
                    print(f"[ARDUINO] {line}")

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[STOPPED] Attendance logging session terminated by user.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()
