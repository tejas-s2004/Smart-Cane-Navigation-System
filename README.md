# Smart-Cane-Navigation-System
An ESP32-based navigation cane for the visually impaired integrating RFID tracking and ultrasonic obstacle safety detection.
# Smart Foldable Cane Navigation System

An assistive IoT device designed to enhance independent mobility for the visually impaired. This system balances real-time environmental awareness and indoor localization by employing non-blocking processing loops on an ESP32 microcontroller.

## 🎥 Project Demonstration
---

## 🛠️ System Features
* **Non-Blocking Obstacle Detection:** Utilizes an HC-SR04 ultrasonic sensor running on a dedicated hardware timer interval (150ms) to ensure continuous monitoring without stalling navigation routines or audio streams.
* **RFID Indoor Localization:** Features an MFRC522 RFID module configured over a noise-stabilized SPI bus to scan floor markers and instantly map coordinates to location-specific directions.
* **Real-Time Voice Interface:** Employs a DFPlayer Mini module over Hardware Serial (`UART2`) to deliver immediate audio announcements and hazard beep alerts directly through a headset.

## 📐 Hardware Architecture & Safety Adjustments
* **Microcontroller:** ESP32 DevKit V1 (Core processing logic).
* **Power Regulation:** MT3608 Boost Converter delivering a dedicated, stable 5V rail for high-pulse ultrasonic pings and audio amplification.
* **Signal Protection:** An integrated 3x 1kΩ resistor voltage divider framework calibrated to step down the ultrasonic 5V Echo pulse safely to ESP32-compatible 3.3V logic levels.
