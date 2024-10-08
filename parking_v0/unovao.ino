#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SerialCommand.h>

#define RST_PIN 5  // Đổi chân RST để tránh xung đột
#define SS_PIN  10 

MFRC522 mfrc522(SS_PIN, RST_PIN);
String card[13] = {"2ADBF93D", "B18BFC3D", "44DCF93D", "9C93F93D", "D3F4F93D", "EFF2FB3D", "CFF2FB3D", "508CFC3D", "3322FC3D", "135D6A19", "13FD7CE2", "23837DE4", "A965FC3D"};
String tagID = "";

Servo servovao;

SerialCommand sCmd;

#define irEnter 2

void servo_open() {
  char *arg;
  arg = sCmd.next();
  char value = atoi(arg);
  servovao.write(arg);
  delay(2000);
  servovao.write(0);
}

void servo_close() {
  char *arg;
  arg = sCmd.next();
  char value = atoi(arg);
  servovao.write(0);
}

void readsensor() {
  ENTER = digitalRead(irEnter) ? 0 : 1;
}

void setup() {
    Serial.begin(9600);  // Initialize serial communications with the PC

    while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();         // Init SPI bus
    mfrc522.PCD_Init();  // Init MFRC522

    servovao.attach(3);  // Gán chân servo không trùng với RST_PIN
    servovao.write(0);
    Serial.println("Hello");

    sCmd.addCommand("SERVO", servo_open);

    pinMode(irEnter, INPUT);
}

void loop() {
    sCmd.readSerial();
    if (getID()) {
        bool cardFound = false;
        unsigned long uid = getID();
        for (int i = 0; i < 13; i++) {
            if (tagID == card[i]) {
                Serial.print("Card detected, UID: "); Serial.println(uid);
                Serial.println("CARD1 1");
                cardFound = true;           // Xác định thẻ đã tìm thấy
                break;                      // Thoát khỏi vòng lặp
            }
        }
        if (!cardFound) {  // Nếu thẻ không được tìm thấy
            Serial.println("No card found");
            Serial.println("CARD1 0");
        }
    }
    if (digitalRead(irEnter) == 1) {
      Serial.println("ENTER 1");
    } else if (digitalRead(irEnter) == 0) {
      Serial.println("ENTER 0");
    }
}

boolean getID() {
    if (!mfrc522.PICC_IsNewCardPresent()) {  // If a new PICC placed to RFID reader continue
        return false;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {  // Since a PICC placed get Serial and continue
        return false;
    }
    tagID = "";
    for (uint8_t i = 0; i < 4; i++) {
        tagID.concat(String(mfrc522.uid.uidByte[i], HEX));  // Adds the 4 bytes in a single String variable
    }
    tagID.toUpperCase();
    mfrc522.PICC_HaltA();  // Stop reading
    return true;
}

unsigned long getUID(){
  if (!mfrc522.PICC_IsNewCardPresent()) {  // Kiểm tra có thẻ mới không
    return 0;
  }
  
  if (!mfrc522.PICC_ReadCardSerial()) {  // Đọc thông tin thẻ
    return 0;
  }
  
  unsigned long hex_num = 0;
  
  // Nếu UID là 4 byte thì xử lý theo kiểu này
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];

  mfrc522.PICC_HaltA();  // Dừng đọc thẻ
  return hex_num;
}