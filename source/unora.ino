#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>

#define RST_PIN 5  // Đổi chân RST để tránh xung đột
#define SS_PIN  10 

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Đối tượng điều khiển module RFID
String cards[13] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", 
                   "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", 
                   "13FD7CE2", "23837DE4", "A965FC3D"};
String tagId = "";

Servo exitServo;  // Servo để mở cửa

int exitStatus = 0;
int enterStatus = 0;

void setup() {
    Serial.begin(9600);  // Khởi tạo giao tiếp Serial
    
    SPI.begin();  // Khởi tạo giao tiếp SPI
    mfrc522.PCD_Init();  // Khởi tạo module RFID

    exitServo.attach(9);  // Gán servo vào chân số 3
    exitServo.write(0);   // Đưa servo về vị trí 0 độ (đóng cửa)

    Serial.println("Hello2");  // In ra thông báo khởi động thành công
}

void loop() {
    if (isCardDetected()) {
        bool isCardFound = false;
        for (int i = 0; i < 13; i++) {
            if (tagId == cards[i]) {
                Serial.print("Card detected, UID: ");
                Serial.println(tagId);
                openGate();
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
void openGate() {
    exitServo.write(90);  // Xoay servo 90 độ (mở cửa)
    delay(2000);         // Giữ cửa mở trong 2 giây
    exitServo.write(0);   // Đóng cửa lại
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

    // Tạo một mảng char để lưu UID dạng chuỗi
    char uidStr[9];  // Mỗi byte của UID có 2 ký tự hex, 4 bytes -> 8 ký tự + null terminator
    for (uint8_t i = 0; i < 4; i++) {
        sprintf(&uidStr[i * 2], "%02X", mfrc522.uid.uidByte[i]);  // Lưu từng byte UID vào chuỗi dạng HEX
    }
    tagId = String(uidStr);  // Chuyển thành String để sử dụng

    tagId.toUpperCase();  // Đưa chuỗi thành chữ hoa

    mfrc522.PICC_HaltA();  // Dừng việc đọc thẻ

    return true;
}
