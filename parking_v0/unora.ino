#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <SerialCommand.h>


#define RST_PIN 5  // Đổi chân RST để tránh xung đột
#define SS_PIN  10 

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Đối tượng điều khiển module RFID
String card[13] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", 
                   "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", 
                   "13FD7CE2", "23837DE4", "A965FC3D"};
String tagID = "";

Servo servora;  // Servo để mở cửa

int exitStat = 0;
int enterStat = 0;

SerialCommand sCmd;

void setup() {
    Serial.begin(9600);  // Khởi tạo giao tiếp Serial
    
    SPI.begin();  // Khởi tạo giao tiếp SPI
    mfrc522.PCD_Init();  // Khởi tạo module RFID

    servora.attach(9);  // Gán servo vào chân số 3
    servora.write(0);   // Đưa servo về vị trí 0 độ (đóng cửa)

    Serial.println("Hello2");  // In ra thông báo khởi động thành công
}

void loop() {
    sCmd.readSerial();
    if (getID()) {
        bool cardFound = false;
        for (int i = 0; i < 13; i++) {
            if (tagID == card[i]) {
                Serial.print("Card detected, UID: ");
                Serial.println(tagID);
                opengate();
                cardFound = true;
                break;
            }
        }

        if (!cardFound) {  // Nếu không tìm thấy thẻ hợp lệ
            Serial.println("No card found");
            delay(100);
        }
    }
}

// Hàm mở cửa
void opengate() {
    servora.write(90);  // Xoay servo 90 độ (mở cửa)
    delay(2000);         // Giữ cửa mở trong 2 giây
    servora.write(0);   // Đóng cửa lại
}


// Hàm đọc và lưu UID của thẻ RFID
boolean getID() {
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
    tagID = String(uidStr);  // Chuyển thành String để sử dụng

    tagID.toUpperCase();  // Đưa chuỗi thành chữ hoa

    mfrc522.PICC_HaltA();  // Dừng việc đọc thẻ

    return true;
}
