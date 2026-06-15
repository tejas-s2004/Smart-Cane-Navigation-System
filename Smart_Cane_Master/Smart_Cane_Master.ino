#include <SPI.h>
#include <MFRC522.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

// -------------------------------------------------------------
// Pin Configurations
// -------------------------------------------------------------
// RFID MFRC522 SPI Pins
#define RST_PIN         22          
#define SS_PIN          21          
MFRC522 mfrc522(SS_PIN, RST_PIN);

// DFPlayer Mini UART Pins (Hardware Serial 2)
#define RXD2 16  // Connects to DFPlayer TX
#define TXD2 17  // Connects to DFPlayer RX via 1k ohm resistor
HardwareSerial mySerial(2);
DFRobotDFPlayerMini myDFPlayer;

// HC-SR04 Ultrasonic Sensor Pins
#define TRIG_PIN        32  
#define ECHO_PIN        35  // Protected via your 3x 1k ohm voltage divider

// -------------------------------------------------------------
// Target Location Track Database (Mapped to your exact UIDs)
// -------------------------------------------------------------
// Active Tags
const byte tagLocation2[]  = {0x2A, 0x13, 0x7B, 0x06}; // Will play 0002.mp3
const byte tagLocation3[]  = {0x35, 0x70, 0x7A, 0x06}; // Will play 0003.mp3
const byte tagLocation4[]  = {0xA3, 0xAF, 0x7C, 0x06}; // Will play 0001.mp3 (Re-routing Tag 4 here)
const byte tagLocation5[]  = {0x29, 0x58, 0x7E, 0x06}; // Will play 0005.mp3

// Standby Tags (Currently ignored per instructions)
// const byte tagLocation1[] = {0x45, 0x58, 0x4F, 0x06}; 
// const byte tagCard[]      = {0x62, 0xB2, 0xDC, 0x5C}; 

// Obstacle Detection Settings
const int obstacleThresholdCm = 50;  // Alert distance threshold (50cm)
unsigned long lastUltrasonicScan = 0;
const unsigned long ultrasonicInterval = 150; // Scan loop rate in milliseconds

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for hardware rails to settle completely

  Serial.println(F("\n=================================================="));
  Serial.println(F("---     MASTER CANE FIRMWARE: FINAL TAG MAP     ---"));
  Serial.println(F("=================================================="));

  // Initialize Ultrasonic Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW); 

  // Initialize SPI Bus with stabilized clock speed for soldered traces
  SPI.begin(18, 19, 23, SS_PIN); 
  SPI.setClockDivider(SPI_CLOCK_DIV16); 
  mfrc522.PCD_Init();
  delay(10); 
  Serial.println(F("[SUCCESS] RFID MFRC522 Synchronized."));

  // Initialize Hardware Serial 2 for Audio Delivery
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  unsigned long startAttemptTime = millis();
  bool audioConnected = false;
  
  while (millis() - startAttemptTime < 4000) { 
    if (myDFPlayer.begin(mySerial)) {
      audioConnected = true;
      break;
    }
    delay(500);
    Serial.print(F("."));
  }

  if (!audioConnected) {
    Serial.println(F("\n[ERROR] Audio Module Offline. Check power traces."));
  } else {
    Serial.println(F("\n[SUCCESS] DFPlayer Mini Connected."));
    myDFPlayer.readType(); 
    myDFPlayer.volume(22); // Optimized headset volume  
    delay(500);
  }

  Serial.println(F("\n>>> System Active: Walking Protection Mode Engaged."));
}

void loop() {
  // -------------------------------------------------------------
  // PART 1: ULTRASONIC OBSTACLE SCANNING (BEEP = 0004)
  // -------------------------------------------------------------
  unsigned long currentMillis = millis();
  if (currentMillis - lastUltrasonicScan >= ultrasonicInterval) {
    lastUltrasonicScan = currentMillis;
    
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout prevents freeze
    int distanceCm = duration * 0.034 / 2;
    
    if (distanceCm > 0 && distanceCm <= obstacleThresholdCm) {
      Serial.print(F("[ALERT] Obstacle Close: "));
      Serial.print(distanceCm);
      Serial.println(F(" cm. Playing Beep Track 0004.mp3..."));
      
      myDFPlayer.playMp3Folder(4); // Ultrasonic beep
    }
  }

  // -------------------------------------------------------------
  // PART 2: RFID LOCATION MARKER TRACKING
  // -------------------------------------------------------------
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("\n--- TAG CAPTURED ---"));
  Serial.print(F("Marker UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Initialize match tracking flags
  bool match2 = true;
  bool match3 = true;
  bool match4 = true;
  bool match5 = true;

  for (byte i = 0; i < 4; i++) {
    if (mfrc522.uid.uidByte[i] != tagLocation2[i]) match2 = false;
    if (mfrc522.uid.uidByte[i] != tagLocation3[i]) match3 = false;
    if (mfrc522.uid.uidByte[i] != tagLocation4[i]) match4 = false;
    if (mfrc522.uid.uidByte[i] != tagLocation5[i]) match5 = false;
  }

  // Route to specific location tracks
  if (match4) {
    Serial.println(F("TAG 4 MATCH -> Playing Track 0001.mp3"));
    myDFPlayer.playMp3Folder(1); 
  } 
  else if (match2) {
    Serial.println(F("TAG 2 MATCH -> Playing Track 0002.mp3"));
    myDFPlayer.playMp3Folder(2); 
  } 
  else if (match3) {
    Serial.println(F("TAG 3 MATCH -> Playing Track 0003.mp3"));
    myDFPlayer.playMp3Folder(3); 
  } 
  else if (match5) {
    Serial.println(F("TAG 5 MATCH -> Playing Track 0005.mp3"));
    myDFPlayer.playMp3Folder(5); 
  } 
  else {
    Serial.println(F("TAG SCANNED BUT CURRENTLY IGNORED/UNMAPPED."));
  }

  mfrc522.PICC_HaltA();
  delay(3000); // 3-second system cooldown matching voice duration
}