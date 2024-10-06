#include <Wire.h>
#include <Servo.h>

int servoPin = 9;
Servo servo;

void setup() {
  Serial.begin(38400);
  Wire.begin(6); // Khởi tạo thư viện I2C với địa chỉ 6
  Wire.onReceive(receiveEvent); // Khởi tạo chế độ nhận tín hiệu từ boad chủ
  servo.attach(servoPin);
  servo.write(0); // Đặt servo ở vị trí 90 độ ban đầu
}

void loop() {
 
}

void receiveEvent() { // Hàm sự kiện nhận tín hiệu từ boad chủ
  while (Wire.available()) { // Chờ cho đến khi có tín hiệu
    char c = Wire.read(); // Biến c để lưu dữ liệu nhận được
    if (c == 'i') { // Nếu boad chủ gửi về tín hiệu là 'I'
      Serial.println("i");
      servo.write(90); // Di chuyển servo đến 90 độ
      delay(1000); // Duy trì 90 độ trong 1 giây
      servo.write(0); // Di chuyển servo về 0 độ
    } else if (c == 'o') {
      Serial.println("o");
      servo.write(0); // Di chuyển servo về 0 độ
    }
  }
}
