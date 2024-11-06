#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>

#define RST_PIN 5
#define SS_PIN  10 
#define SERVO_PIN 9
#define SERVO_OPEN 90
#define SERVO_CLOSE 0

MFRC522 mfrc522(SS_PIN, RST_PIN); 
String cardList[13] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", 
                       "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", 
                       "13FD7CE2", "23837DE4", "A965FC3D"};
String tagId = "";

Servo exitServo;

int exitStatus = 0;
int enterStatus = 0;

void setup() {
    Serial.begin(9600); // Serial begin
    
    SPI.begin();  // Khởi tạo giao tiếp SPI
    mfrc522.PCD_Init();  // Khởi tạo module RFID
    // Check if RFID is responding
    if (!mfrc522.PCD_PerformSelfTest()) {
        Serial.println("RFID Exit initialization failed");
        // You might want to add some error indication on the LCD here
    } else {
        Serial.println("RFID Exit initialized successfully");
    }
    Serial.println("Exit");

    exitServo.attach(SERVO_PIN);  // Gán servo vào chân số 3
    exitServo.write(SERVO_CLOSE);   // Đưa servo về vị trí 0 độ (đóng cửa)
}

void loop() {
    if (isCardDetected()) {
        bool isCardFound = false;
        for (int i = 0; i < 13; i++) {
            if (tagId == cardList[i]) {
                Serial.print("Card detected, UID: ");
                Serial.println(tagId);
                doOpenGate();
                isCardFound = true;
                break;
            }
        }

        if (!isCardFound) {  // Nếu không tìm thấy thẻ hợp lệ
            Serial.println("No card found");
            delay(100);
        }
    }
}

// Hàm mở cửa
void doOpenGate() {
    exitServo.write(SERVO_OPEN);  // Xoay servo 90 độ (mở cửa)
    delay(2000);         // Giữ cửa mở trong 2 giây
    exitServo.write(SERVO_CLOSE);   // Đóng cửa lại
}


// Hàm đọc và lưu UID của thẻ RFID
boolean isCardDetected() {
    // Kiểm tra xem có thẻ mới không
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }

    // Đọc UID của thẻ nếu có
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }

    // Create char array to save UID in String 
    char uidStr[9]; // Each byte of UID has 2 hex characters, 4 bytes -> 8 characters + null terminator
    for (uint8_t i = 0; i < 4 && i * 2 < sizeof(uidStr) - 1; i++) {
        sprintf(&uidStr[i * 2], "%02X", mfrc522.uid.uidByte[i]);
    }
    uidStr[8] = '\0';  // Ensure null termination

    tagId = String(uidStr);  // Convert to String
    tagId.toUpperCase();  // Upper case
    mfrc522.PICC_HaltA();  // Stop reading card
    return true;
}
