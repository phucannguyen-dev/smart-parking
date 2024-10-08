#include <Servo.h> // Includes the servo library
#include <Wire.h> 
#include <LCDI2C_Multilingual.h>
#include <SerialCommand.h>

LCDI2C_Vietnamese lcd(0x27, 20, 4); // Setup man hinh

// Setup cam bien slot tu 1 -> 6
#define irCar1 53
#define irCar2 52
#define irCar3 51
#define irCar4 50
#define irCar5 49
#define irCar6 48

// Trang thai slot
int S1 = 0, S2 = 0, S3 = 0, S4 = 0, S5 = 0, S6 = 0;

// Trang thai cua ir vao / ra
int enterStat = 0, exitStat = 0;

unsigned long lastCheck = 0; // Lan kiem tra cuoi cung
const long interval = 500; // Kiem tra cam bien moi 500ms

int isCardVao = 0;
int isCardRa = 0;

SerialCommand sCmd;

void cardvao() { 
  //Đoạn code này dùng để đọc TỪNG tham số. Các tham số mặc định có kiểu dữ liệu là "chuỗi"
  char *arg;
  arg = sCmd.next();
  int value = atoi(arg); // Chuyển chuỗi thành số
  isCardVao = value;
}

void cardra() { 
  //Đoạn code này dùng để đọc TỪNG tham số. Các tham số mặc định có kiểu dữ liệu là "chuỗi"
  char *arg;
  arg = sCmd.next();
  int value = atoi(arg); // Chuyển chuỗi thành số
  isCardRa = value;
}

void irvao() {
  char *arg;
  arg = sCmd.next();
  int value = atoi(arg);
  enterStat = value;
}

void irra() {
  char *arg;
  arg = sCmd.next();
  int value = atoi(arg);
  exitStat = value;
}

// Trang thai ir slot
void Read_Sensor() {
    S1 = digitalRead(irCar1) ? 0 : 1;
    S2 = digitalRead(irCar2) ? 0 : 1;  
    S3 = digitalRead(irCar3) ? 0 : 1;
    S4 = digitalRead(irCar4) ? 0 : 1;
    S5 = digitalRead(irCar5) ? 0 : 1;
    S6 = digitalRead(irCar6) ? 0 : 1;
}

void setup() {
    // Khai bao pin
    pinMode(irCar1, INPUT);
    pinMode(irCar2, INPUT);
    pinMode(irCar3, INPUT);
    pinMode(irCar4, INPUT);
    pinMode(irCar5, INPUT);
    pinMode(irCar6, INPUT);
    pinMode(irEnter, INPUT);
    pinMode(irExit, INPUT);

    Serial1.begin(9600); // Xe Vao
    Serial2.begin(19200); // Xe Ra

    sCmd.addCommand("CARD1", cardvao);
    sCmd.addCommand("CARD2", cardra);
    sCmd.addCommand("ENTER", irvao);
    sCmd.addCommand("EXIT", irra);

    // Setup man hinh lan dau
    lcd.init();
    lcd.backlight();

    // In man hinh chao mung
    lcd.setCursor(5, 1);
    lcd.print("Bãi giữ xe");
    lcd.setCursor(4, 2);
    lcd.print("THCS Tân Sơn");
    delay(2000);
    lcd.clear();

    // Doc cam bien
    Read_Sensor();
}

bool sensorPrinted = false;

void loop() {
    if (millis() - lastCheck >= interval) { 
        lastCheck = millis(); 
        Read_Sensor();

        int totalSlot = S1 + S2 + S3 + S4 + S5 + S6;
        int slot = 6 - totalSlot;

        sCmd.readSerial();

        // Cập nhật LCD nếu có thay đổi slot
        static int lastSlot = -1;
        if (slot != lastSlot) {
            lcd.setCursor(0, 0);
            lcd.print("   Số chỗ trống: ");
            lcd.print(slot);
            lcd.print("  "); 
            lastSlot = slot;
        }

        if (slot == 0) {
            lcd.setCursor(0, 0);
            lcd.print("                  ");
            lcd.setCursor(2, 0);
            lcd.print("Bãi xe đã đầy");
        }

        // Cập nhật trạng thái của các slot
        updateLCD(1, S1, 0, 1);
        updateLCD(2, S2, 10, 1);
        updateLCD(3, S3, 0, 2);
        updateLCD(4, S4, 10, 2);
        updateLCD(5, S5, 0, 3);
        updateLCD(6, S6, 10, 3);

        handleGate(slot, isCardVao, isCardRa); // Xử lý servo
    }
}

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

bool exitGateOpen = false;  // Cờ trạng thái cho cổng ra

void handleGate(int &slot, int &isCardVao, int &isCardRa, int &enterStat, int &exitStat) {
  // Mở cổng vào nếu còn slot và phát hiện xe vào
  if (slot > 0 && enterStat == 1 && exitStat == 0 && isCardVao == 1) {
    slot -= 1;
    Serial1.println("SERVO 90"); // 1 vao, 2 ra, 90 mo, 0 dong
  } 
  
  // Mở cổng ra nếu phát hiện xe ra
  if (exitStat == 1 && enterStat == 0 && isCardRa == 1) {
    slot += 1;
    Serial2.println("SERVO 90");
  }

  // Trường hợp bãi đầy nhưng vẫn phát hiện xe ra
  if (slot == 0 && exitStat == 1) {
    slot += 1; // Tăng số slot trống khi xe ra
    Serial2.println("SERVO 90");
  }
}
