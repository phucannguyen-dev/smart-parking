// Version 2. Copyright by Nguyen Phuc An, Hoang Tan Dat, Vu Xuan Hoa. 17:10 02/11/24
// Variables Refactor
/*
Booleans: isSomething
Integers: something
Strings: something
const: SOMETHING
Functions: doSomething
*/

// Library
#include <Servo.h>
#include <Wire.h>
#include <LCDI2C_Multilingual.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

// Declare
Servo entranceServo; // Control entrance gate
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
const int LCD_SLOT_STATUS_ROW = 0;
const int LCD_FIRST_COL = 0;
const int LCD_SECOND_COL = 10;

const int SERVO_PIN = 9;
const int SERVO_OPEN_ANGLE = 90;
const int SERVO_CLOSE_ANGLE = 0;

const int SERIAL_BAUD = 9600;

const int TOTAL_SLOTS = 6;
const int CARD_COUNT = 13;

// EEPROM Configuration
const int CARDS_START_ADDR = 0;  // Starting address for card storage
const int CARD_LENGTH = 8;  // Length of each card ID (8 characters)
const int MAX_CARDS = 50;  // Maximum number of cards
const int ACTIVE_CARDS_ADDR = CARDS_START_ADDR + (MAX_CARDS * CARD_LENGTH);  // Address to store number of active cards
// Global variables
int activeCards = 0;  // Declare this globally

LCDI2C_Vietnamese lcd(0x27, LCD_COLS, LCD_ROWS); // LCD display

// Sensors
#define IR_CAR1 30
#define IR_CAR2 31
#define IR_CAR3 32
#define IR_CAR4 33
#define IR_CAR5 34
#define IR_CAR6 35

// RFID
#define RST_PIN 5
#define SS_PIN 53
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Card IDs declare
String card[CARD_COUNT] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", 
                   "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", 
                   "13FD7CE2", "23837DE4", "A965FC3D"};
String tagID = "";


// Slot status
int s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0, s6 = 0;
int totalSlot = s1 + s2 + s3 + s4 + s5 + s6;
int slot = TOTAL_SLOTS - totalSlot;

// Check time between sensor check.
unsigned long lastCheck = 0;
unsigned long lastCheckCard = 0;
const long INTERVAL = 500; // Default delay


// Read sensors status
void doReadSensor() {
    s1 = digitalRead(IR_CAR1) ? 0 : 1;
    s2 = digitalRead(IR_CAR2) ? 0 : 1;
    s3 = digitalRead(IR_CAR3) ? 0 : 1;
    s4 = digitalRead(IR_CAR4) ? 0 : 1;
    s5 = digitalRead(IR_CAR5) ? 0 : 1;
    s6 = digitalRead(IR_CAR6) ? 0 : 1;

    totalSlot = s1 + s2 + s3 + s4 + s5 + s6; // Total slot

    // Empty slot
    slot = TOTAL_SLOTS - totalSlot;
}


// First setup
void setup() {
    // Set sensors to INPUT mode
    pinMode(IR_CAR1, INPUT);
    pinMode(IR_CAR2, INPUT);
    pinMode(IR_CAR3, INPUT);
    pinMode(IR_CAR4, INPUT);
    pinMode(IR_CAR5, INPUT);
    pinMode(IR_CAR6, INPUT);

    Serial.begin(SERIAL_BAUD); // Begin Serial Monitor
    doInitializeEEPROM();  // Initialize EEPROM
    SPI.begin();         // Init SPI bus
    mfrc522.PCD_Init();  // Init MFRC522
    // Check if RFID is responding
    if (!mfrc522.PCD_PerformSelfTest()) {
        Serial.println("RFID initialization failed");
        // You might want to add some error indication on the LCD here
    } else {
        Serial.println("RFID initialized successfully");
    }
    Serial.println("Entrance");

    // Setup display
    lcd.init();
    lcd.backlight();
    Wire.begin();
    Wire.beginTransmission(0x27); // LCD I2C address
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("LCD found at 0x27");
        lcd.init();
        lcd.backlight();
    } else {
        Serial.println("LCD not found or not responding");
    }

    // Setup entrance servo
    entranceServo.attach(SERVO_PIN);
    entranceServo.write(SERVO_CLOSE_ANGLE); // Close gate
    
    // Display welcome messages
    lcd.setCursor(5, 1);
    lcd.print("Bãi giữ xe");
    lcd.setCursor(4, 2);
    lcd.print("THCS Tân Sơn");
    delay(2000);
    lcd.clear();

    doReadSensor();  // Đọc trạng thái cảm biến lần đầu tiên
}


// GATE
unsigned long gateOpenTime = 0;
const unsigned long GATE_OPEN_DURATION = 2000; // Default gate delay
// Open gate function
void doOpenGate() {
    entranceServo.write(SERVO_OPEN_ANGLE); // Open
    gateOpenTime = millis();
}

void doCheckGateStatus() {
    if (gateOpenTime > 0 && millis() - gateOpenTime >= GATE_OPEN_DURATION) {
        entranceServo.write(SERVO_CLOSE_ANGLE);
        gateOpenTime = 0;
    }
}


// RFID
// Read and return UID
boolean isGetID() {
    // Check for new card
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }

    // Read UID (if it has)
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    // Create char array to save UID in String 
    char uidStr[9]; // Each byte of UID has 2 hex characters, 4 bytes -> 8 characters + null terminator
    for (uint8_t i = 0; i < 4 && i * 2 < sizeof(uidStr) - 1; i++) {
        sprintf(&uidStr[i * 2], "%02X", mfrc522.uid.uidByte[i]);
    }
    uidStr[8] = '\0';  // Ensure null termination

    tagID = String(uidStr);  // Convert to String
    tagID.toUpperCase();  // Upper case
    mfrc522.PICC_HaltA();  // Stop reading card
    return true;
}

void doReadCard() {
  // Set reading card time each 500ms
  if (millis() - lastCheckCard >= INTERVAL) {
    lastCheckCard = millis();
    if (isGetID()) { // If found RFID card
        bool isCardFound = false; // Default is false
        for (int i = 0; i < CARD_COUNT; i++) {
            if (tagID == card[i]) { // If the card's ID is the same with one of IDs in card array.
                Serial.print("Card detected, UID: ");
                Serial.println(tagID);
                if (slot != 0) {
                    doOpenGate(); // Open gate
                }
                isCardFound = true;
                break;
            }
        }
        if (!isCardFound) {  // If don't found any card
            Serial.println("No card found");
        }
    }
  }
}

/**
 * Initialize EEPROM and load saved cards
 */
void doInitializeEEPROM() {
    // Read number of active cards
    activeCards = EEPROM.read(ACTIVE_CARDS_ADDR);
    if (activeCards > MAX_CARDS) activeCards = 0;  // Safety check
    
    // Load cards from EEPROM
    for (int i = 0; i < activeCards; i++) {
        card[i] = doReadCardFromEEPROM(i);
    }
    
    Serial.print("Loaded cards: ");
    Serial.println(activeCards);
}

/**
 * Read a card ID from EEPROM at specific index
 */
String doReadCardFromEEPROM(int index) {
    String cardID = "";
    int addr = CARDS_START_ADDR + (index * CARD_LENGTH);
    
    for (int i = 0; i < CARD_LENGTH; i++) {
        char c = EEPROM.read(addr + i);
        if (c != 0) cardID += c;
    }
    
    return cardID;
}

/**
 * Write a card ID to EEPROM at specific index
 */
void doWriteCardToEEPROM(int index, String cardID) {
    int addr = CARDS_START_ADDR + (index * CARD_LENGTH);
    
    // Write card ID
    for (int i = 0; i < CARD_LENGTH; i++) {
        if (i < cardID.length()) {
            EEPROM.update(addr + i, cardID[i]);  // Using update instead of write to reduce wear
        } else {
            EEPROM.update(addr + i, 0);
        }
    }
    
    // Update active cards count
    EEPROM.update(ACTIVE_CARDS_ADDR, activeCards);
}

/**
 * Add a new card to EEPROM
 */
boolean doAddCard(String newCard) {
    if (activeCards >= MAX_CARDS) {
        Serial.println("Card storage full");
        return false;
    }
    
    // Check if card already exists
    for (int i = 0; i < activeCards; i++) {
        if (card[i] == newCard) {
            Serial.println("Card already exists");
            return false;
        }
    }
    
    // Add new card
    card[activeCards] = newCard;
    doWriteCardToEEPROM(activeCards, newCard);
    activeCards++;
    EEPROM.update(ACTIVE_CARDS_ADDR, activeCards);
    
    Serial.print("Added card: ");
    Serial.println(newCard);
    return true;
}

/**
 * Delete a card from EEPROM
 */
boolean doDeleteCard(String cardToDelete) {
    bool isDeleted = false;
    
    for (int i = 0; i < activeCards; i++) {
        if (card[i] == cardToDelete) {
            // Shift remaining cards
            for (int j = i; j < activeCards - 1; j++) {
                card[j] = card[j + 1];
                doWriteCardToEEPROM(j, card[j]);
            }
            activeCards--;
            EEPROM.update(ACTIVE_CARDS_ADDR, activeCards);
            
            isDeleted = true;
            Serial.print("Deleted card: ");
            Serial.println(cardToDelete);
            break;
        }
    }
    
    if (!isDeleted) {
        Serial.println("Card not found");
    }
    
    return isDeleted;
}

/**
 * Handle serial commands for card management
 */
 bool isEmergencyMode = false;
void doCheckSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.startsWith("ADD:")) {
            String cardID = command.substring(4);
            cardID.trim();
            doAddCard(cardID);
        }
        else if (command.startsWith("DELETE:")) {
            String cardID = command.substring(7);
            cardID.trim();
            doDeleteCard(cardID);
        }
        else if (command == "LIST") {
            Serial.print("Active cards: ");
            Serial.println(activeCards);
            for (int i = 0; i < activeCards; i++) {
                Serial.print(i);
                Serial.print(": ");
                Serial.println(card[i]);
            }
        }
        if (command == "EMERGENCY") {
            isEmergencyMode = true;
            // Open gate
            entranceServo.write(SERVO_OPEN_ANGLE);
            
            // Update display
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TRƯỜNG HỢP KHẨN CẤP");
            lcd.setCursor(0, 0);
            lcd.print("NGUY HIỂM");
            
            Serial.println("EMERGENCY MODE ACTIVATED");
            Serial.println("All gates opened");
        }
        else if (command == "RESET") {
            isEmergencyMode = false;
            // Close gate
            entranceServo.write(SERVO_CLOSE_ANGLE);
            
            lcd.clear();
            Serial.println("Emergency mode deactivated");
            Serial.println("System returning to normal operation");
        }
    }
}

// DO CODE
void loop() {
    if (millis() - lastCheck >= INTERVAL) { 
        lastCheck = millis(); 
        doReadSensor();  // Update sensor status

        if (!isEmergencyMode) {
            // Update empty slot on the display
            static int lastSlot = -1;

            // Update the display based on the number of available slots
            if (slot != lastSlot) {
                lcd.setCursor(0, 0);
                lcd.print("   Số chỗ trống: ");
                lcd.print(slot);
                lcd.print("  ");
                lastSlot = slot;
            }

            // Update slot status on the display
            doUpdateLCD(1, s1, 0, 1);
            doUpdateLCD(2, s2, 10, 1);
            doUpdateLCD(3, s3, 0, 2);
            doUpdateLCD(4, s4, 10, 2);
            doUpdateLCD(5, s5, 0, 3);
            doUpdateLCD(6, s6, 10, 3);

            doReadCard();
            doCheckGateStatus();
        }
        doCheckSerialCommands(); // Add this line
    }
}

// Function to update slot status on the LCD
void doUpdateLCD(int slotNum, int sensorVal, int col, int row) {
    static int lastState[6] = { -1, -1, -1, -1, -1, -1 };
    if (slotNum < 1 || slotNum > 6) {
        return;
    }
    if (sensorVal != lastState[slotNum - 1]) {
        lcd.setCursor(col, row);
        if ((sensorVal != 0) && (sensorVal != 1)) {
            lcd.print("S" + String(slotNum) + ":Lỗi");
        } else {
            lcd.print(sensorVal == 1 ? "S" + String(slotNum) + ":Có xe" : "S" + String(slotNum) + ":Trống");
        }
        lastState[slotNum - 1] = sensorVal;
    }
}
