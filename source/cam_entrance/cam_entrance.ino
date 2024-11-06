#include <SoftwareSerial.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>
// define the number of bytes you want to access
#define EEPROM_SIZE 1
 
RTC_DATA_ATTR int bootCount = 0;

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define WIFI_SSID "Redmi12"
#define WIFI_PASSWORD "0123456789"

// Google Project ID
#define PROJECT_ID "smartparking-datalogging"

// Service Account's client email
#define CLIENT_EMAIL "smartparking-datalogging@smartparking-datalogging.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCcqXs8G7h4KKcx\nIF9ovJ1wpFmJvDHgQUL2J/OwDSuzHoni+SVQrGBfgIPDcfvp50OzoqXJEVm/RHUP\nf//FQ+iL5F2peUoxLy9kA4ULmp2yUlBGuqd2ahn2LYjIvAqDUiO1Cl/y41XTxY8b\nw2/MPwRfRHyCFLCVWboaqg/EOuQeaID8mcpCwq4XwaKB3CnNzTaZCuNK1BAkbPB7\nMk5Wu2HADxqK+q62vUUnM6cPHnv+cPbqZ79PE3QrKWZ72lFrATbvYrPPlJBCd8Np\nLK1wQVnrtRKxUa11by1V4F5NxPxIZCj0BrDOIld0MZlQP6nEQti4T5Xk4gCQuQgk\n0cIp7fq1AgMBAAECggEAAZ81kek9jyzokjciFb0CKkThUm462/YBAfhm6vcv8SJL\nTijDLKMkOkjj1FWtRH4q8sSnz72YvsSwK5FpLfsmgLBEEfmZ/YleBbyNl5GpuUtl\n3l2XV0AA9c4B4QYlW7exe/HYDXUyEce1VBXBZg1u96eHXHdnXpx3s0f4ctEjRHdP\nWf13AzjGdxpJ7schJONr0pjlW/Cp6kXg5f/esGteLwVQWNr5YT57EwCULSqOuZkN\njByXf3+17e4qxQT/zLsSTLWbEFA0bgnagYA4VBONMrs5+z9jcs7veZkAGSOAAjpY\nESZWxqpN/9gucUc4ackB/m3bimTdNlkkvtPKj6CcbwKBgQDX7afIP7Vl+7AfiajG\n6kBbLCVkJpM4wghsFJ4VOlx6Al7mk1xCM0adr7eWsAKMGevpTt77i5bGYbnu712g\nWeQ9N7MrauLq0sB11AvxGXzJPjc8fg7PBDt9SnIZKJPAozSXC4n3FaHjnaCBVTVO\nKgAT+SI+pBe46wxU3lCgSO9TBwKBgQC5vDJWRMhadHM4PQj4Gv02ucimQRu7TJZB\nqgPVZMqWGM7Qe+5Sxkx2/V6RIMQRCWP8ADi8LvCJs4P0XZmfn8hCblYCSu+e7jG4\nmlMriA0XpTgk4K5ThT4lL0inkaE8gKSceZh8N//rMr/FXdUV/PSPwnCzaKGuATmW\nbDiLLVVpYwKBgA4K0Ix/SCa3tX53j1hQR5smRdA7ift48t9Ef/tVbkoo+U75aliL\nyR3cHY5ZJ/FAhk0H6gxlyYphNJxLSMVy0xsAElDShKtMxWNkRNqLE8hqmWD/uarQ\n1aWSP9VPBDzU4EiqYXCGIZ8R0yxWBD9vD848t3NnbHuEHasvQgMKezXlAoGBAIQR\nFZnR/sSS2Pq8K4x/ZF1tbQcKdLWRl6XJRkCOEp73tNb5yFSDPAu/zWZUbhXPH5Eo\ndy8YEWFYjBNOGtYtvy+9VNr8fbX8mM4yrcDeVLDjgjB2sn4g2DeIj5jXLFbhAith\nKtPo3chXpZSpS8L+wscV8x0e4RfhNxmGig4shfcRAoGBAK8yPsxiHlsiVXbzZxxM\nMfHTQkvvLN4xrLe4vkvMawhmpsqopY6tZkGkrA+Y6OUnWCVLMqyAcbIniy4zp9SG\ns/+WDFnP7Xx8Qi0kBil8m1lpPDqwEvttOmhVwb4e2B1HJU4BvqBS18mh8MVSdhOX\nUtfWkTxVM7ApAy1Y0W81Lis3\n-----END PRIVATE KEY-----\n";

// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1MFuA1l8hoL-z-rRkSXK4GCfL3OLGubck4_A63QAxHCo";

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;

// Token Callback function
void tokenStatusCallback(TokenInfo info);

const char* ntpServer = "pool.ntp.org";
// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

 
int pictureNumber = 0;
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  //Configure time
  configTime(0, 0, ntpServer);

  // Configure GPIO_NUM_13 as input
  pinMode(GPIO_NUM_13, INPUT);

  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);
  // Connect to Wi-Fi
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  pinMode(4, INPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);
 
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA | CIF | VGA | SVGA | XGA | SXGA | UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
 
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
 
  Serial.println("Starting SD Card");
 
  delay(500);
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
 
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
   
  camera_fb_t * fb = NULL;
 
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
 
  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";
 
  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());
 
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb);
  
  delay(1000);
  
  // Turn off ESP32-CAM LED (GPIO 4)
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);
 
  Serial.println("Going to sleep now");
  delay(1000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
} 
 
void loop() {
  bool ready = GSheet.ready();

    if (ready && (millis() - lastTime) > timerDelay){
        lastTime = millis();

        FirebaseJson response;

        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;
        // Create values array for the PIR sensor data
        FirebaseJsonArray arr;
        arr.add(digitalRead(GPIO_NUM_13) == HIGH);  // PIR sensor status
        arr.add(epochTime);  // add timestamp

        valueRange.set("values/[0]", arr);

        // Append values to spreadsheet
        bool success = GSheet.values.append(spreadsheetId, "Sheet1!A1", &valueRange);

        if (success)
          Serial.println("Values appended");
        else
          Serial.println("Failed to append values");
    }
}
