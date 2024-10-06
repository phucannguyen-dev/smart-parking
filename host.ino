#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#define RST_PIN         5          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String card[13] = {"3322FC3D"};  // Danh sách thẻ hợp lệ
String tagID = "";

void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522
  Wire.begin();
  delay(4);            // Optional delay
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
  if (getID()) {
    bool cardFound = false;
    for (int i = 0; i < 13; i++) {
      if (tagID == card[i]) {
        Serial.println("Card found");
        Wire.beginTransmission(6); // Bắt đầu truyền dữ liệu về địa chỉ số 6
        Wire.write('i');            // Truyền ký tự I
        delay(100);
        Wire.write('o');
        Wire.endTransmission();     // Kết thúc truyền dữ liệu
        cardFound = true;           // Xác định thẻ đã tìm thấy
        break;                      // Thoát khỏi vòng lặp
      }
    }
    
    if (!cardFound) {  // Nếu thẻ không được tìm thấy
      Serial.println("No card found");
      Wire.beginTransmission(6);  // Bắt đầu truyền dữ liệu về địa chỉ số 6
      Wire.write('o');             // Truyền ký tự O
      Wire.endTransmission();      // Kết thúc truyền dữ liệu
    }
  }
}

boolean getID() {
  // Getting ready for Reading PICCs
  if (!mfrc522.PICC_IsNewCardPresent()) {  //If a new PICC placed to RFID reader continue
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  //Since a PICC placed get Serial and continue
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
