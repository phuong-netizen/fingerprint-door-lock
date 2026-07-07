#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define RELAY 13

#define C1 17
#define C2 16
#define C3 4
#define C4 2
#define R1 23
#define R2 19
#define R3 18
#define R4 5

#define PASS_LENGTH 4

char correctPass[PASS_LENGTH + 1] = "1234"; 
char inputPass[PASS_LENGTH + 1];

byte passIndex = 0;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {R1, R2, R3, R4};
byte colPins[COLS] = {C1, C2, C3, C4};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);

typedef enum {
    RELAY_OFF = 0,
    RELAY_ON
} RelayState;

typedef enum {
  SCREEN_INIT,
  SCREEN_PASS,
  SCREEN_MENU,
  SCREEN_PIN,
  SCREEN_MANAGE,
  SCREEN_ADD,
  SCREEN_OPEN,
  SCREEN_CHANGE,
  SCREEN_NEWPASS,
  SCREEN_CLIENT,
  SCREEN_STAFF,
  SCREEN_DELETE
} ScreenState;

ScreenState currentScreen = SCREEN_INIT;

void relay_set(RelayState state) {
    if(state == RELAY_ON)
      digitalWrite(RELAY, HIGH);
    else
      digitalWrite(RELAY, LOW);
}
#include "esp_system.h"

void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("[RESET REASON] ");
  switch (reason) {
    case ESP_RST_POWERON:   Serial.println("Power-on (cấp nguồn lần đầu)"); break;
    case ESP_RST_SW:        Serial.println("Software reset (gọi esp_restart/ESP.restart)"); break;
    case ESP_RST_PANIC:     Serial.println("PANIC - crash phần mềm (lỗi code, exception)"); break;
    case ESP_RST_INT_WDT:   Serial.println("Interrupt watchdog timeout"); break;
    case ESP_RST_TASK_WDT:  Serial.println("Task watchdog timeout (code bị treo/block quá lâu)"); break;
    case ESP_RST_WDT:       Serial.println("Watchdog khác"); break;
    case ESP_RST_BROWNOUT:  Serial.println("BROWNOUT - SỤT ÁP NGUỒN"); break;
    case ESP_RST_SDIO:      Serial.println("SDIO reset"); break;
    default:                Serial.printf("Khác: %d\n", reason); break;
  }
}
#include "lcd.h"
#include "control_time.h"
#include "ADD.h"
#include "firebase_sync.h" 
#include "handle_lcd.h" 
#include "handle_finger.h"


// ==========================================
//  TIMER GỬI STATUS ĐỊNH KỲ
// ==========================================
unsigned long lastStatusSend = 0;
#define STATUS_INTERVAL_MS  10000   // Gửi status mỗi 10 giây

void setup() {
  Serial.begin(115200);
  printResetReason();
  Wire.begin(); 
  lcd.begin();
  lcd.backlight();
  lcd_init();
  init_time();
  keypad.setDebounceTime(100);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
  initFingerprint();

  // ==========================================
  //  KHỞI ĐỘNG WIFI (thêm mới)
  // ==========================================
  initWiFi();
  sendIDList();
}

void loop() {
  handleFingerprintUnlock();
  char key = keypad.getKey();

  if (currentScreen == SCREEN_INIT) {
    update_time_LCD();
  }

  // ==========================================
  //  GỬI STATUS ĐỊNH KỲ LÊN FIREBASE (thêm mới)
  // ==========================================
  if (millis() - lastStatusSend >= STATUS_INTERVAL_MS) {
    lastStatusSend = millis();
    sendSystemStatus();
  }

  if(key) {
    switch(currentScreen) {
      case SCREEN_INIT:   handleInit(key);    break;
      case SCREEN_PASS:   handlePass(key);    break;
      case SCREEN_MENU:   handleMenu(key);    break;
      case SCREEN_PIN:    handlePIN(key);     break;
      case SCREEN_MANAGE: handleManage(key);  break;
      case SCREEN_ADD:    handleAdd(key);     break;
      case SCREEN_OPEN:   handleOpen(key);    break;
      case SCREEN_CHANGE: handleChange(key);  break;
      case SCREEN_NEWPASS:handleNewPass(key); break;
      case SCREEN_CLIENT: handleClient(key);  break;
      case SCREEN_STAFF:  handleStaff(key);   break;
      case SCREEN_DELETE: handleDelete(key);  break;
    }
  }
}