#include <Servo.h>
#include <Wire.h>
#include <LCDI2C_Multilingual.h>
#include <SPI.h>
#include <MFRC522.h>

// Khai báo các thiết bị
LCDI2C_Vietnamese lcd(0x27, 20, 4);
Servo servovao; // Servo điều khiển cổng vào

// Cảm biến và chân I/O
#define irCar1 30
#define irCar2 31
#define irCar3 32
#define irCar4 33
#define irCar5 34
#define irCar6 35

// RFID
#define RST_PIN 5
#define SS_PIN 53
MFRC522 mfrc522(SS_PIN, RST_PIN);
String card[13] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", 
                   "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", 
                   "13FD7CE2", "23837DE4", "A965FC3D"};
String tagID = "";

// Trạng thái của các slot
int S1 = 0, S2 = 0, S3 = 0, S4 = 0, S5 = 0, S6 = 0;
int totalSlot = S1 + S2 + S3 + S4 + S5 + S6;
int slot = 6 - totalSlot;
int isCardVao = 0;

// Biến để kiểm tra thời gian giữa các lần quét cảm biến
unsigned long lastCheck = 0;
const long interval = 500;

// Đọc trạng thái cảm biến
void Read_Sensor() {
    S1 = digitalRead(irCar1) ? 0 : 1;
    S2 = digitalRead(irCar2) ? 0 : 1;
    S3 = digitalRead(irCar3) ? 0 : 1;
    S4 = digitalRead(irCar4) ? 0 : 1;
    S5 = digitalRead(irCar5) ? 0 : 1;
    S6 = digitalRead(irCar6) ? 0 : 1;

    totalSlot = S1 + S2 + S3 + S4 + S5 + S6;

    // Tính lại số chỗ trống
    slot = 6 - totalSlot;
}

// Cài đặt ban đầu
void setup() {
    pinMode(irCar1, INPUT);
    pinMode(irCar2, INPUT);
    pinMode(irCar3, INPUT);
    pinMode(irCar4, INPUT);
    pinMode(irCar5, INPUT);
    pinMode(irCar6, INPUT);

    Serial.begin(9600);
    SPI.begin();         // Init SPI bus
    mfrc522.PCD_Init();  // Init MFRC522
    Serial.println("Hello1");

    lcd.init();
    lcd.backlight();

    servovao.attach(9);
    servovao.write(9);
    
    lcd.setCursor(5, 1);
    lcd.print("Bãi giữ xe");
    lcd.setCursor(4, 2);
    lcd.print("THCS Tân Sơn");
    delay(2000);
    lcd.clear();

    Read_Sensor();  // Đọc trạng thái cảm biến lần đầu tiên
}

// Hàm mở cửa
void opengate() {
    servovao.write(90);  // Xoay servo 90 độ (mở cửa)
    delay(2000);         // Giữ cửa mở trong 2 giây
    servovao.write(0);   // Đóng cửa lại
}

void closegate() {
    servovao.write(0);   // Đóng cửa lại
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

void readCard() {
  if (millis() - lastCheck >= interval) {
    lastCheck = millis();
    if (getID()) {
        bool cardFound = false;
        for (int i = 0; i < 13; i++) {
            if (tagID == card[i]) {
                Serial.print("Card detected, UID: ");
                Serial.println(tagID);
                if (slot == 0) closegate();
                else opengate();
                isCardVao = 1;
                cardFound = true;
                break;
            }
        }
        if (!cardFound) {  // Nếu thẻ không được tìm thấy
            Serial.println("No card found");
            isCardVao = 0;
        }
    }
  }
}

void loop() {
    if (millis() - lastCheck >= interval) { 
        lastCheck = millis(); 
        Read_Sensor();  // Cập nhật trạng thái cảm biến
        readCard();

        // Cập nhật số lượng slot còn trống trên màn hình
        static int lastSlot = -1;


        // Nếu bãi xe đã đầy
        if ((slot != lastSlot) && (slot == 0)) {
            lcd.setCursor(2, 0);
            lcd.print("Bãi xe đã đầy");
            lastSlot = slot;
        }

        if (slot != lastSlot) {
            lcd.setCursor(0, 0);
            lcd.print("   Số chỗ trống: ");
            lcd.print(slot);
            lcd.print("  "); 
            lastSlot = slot;
        }

        // Cập nhật trạng thái slot trên màn hình
        updateLCD(1, S1, 0, 1);
        updateLCD(2, S2, 10, 1);
        updateLCD(3, S3, 0, 2);
        updateLCD(4, S4, 10, 2);
        updateLCD(5, S5, 0, 3);
        updateLCD(6, S6, 10, 3);
    }
}

// Cập nhật trạng thái slot trên LCD
void updateLCD(int slotNum, int sensorVal, int col, int row) {
    static int lastState[6] = { -1, -1, -1, -1, -1, -1 };

    if (slotNum < 1 || slotNum > 6) {
        return;
    }

    if (sensorVal != lastState[slotNum - 1]) {
        lcd.setCursor(col, row);
        lcd.print(sensorVal == 1 ? "S" + String(slotNum) + ":Có xe" : "S" + String(slotNum) + ":Trống");
        lastState[slotNum - 1] = sensorVal;
    }
}