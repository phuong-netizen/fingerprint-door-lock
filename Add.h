#ifndef ADD_H
#define ADD_H

#include <Adafruit_Fingerprint.h>
#include <Preferences.h>
#include <RTClib.h> 

// ==========================================
// KẾT NỐI VỚI FILE control_time.h CỦA BẠN
// ==========================================
extern RTC_DS3231 rtc;             // Dùng chung rtc đã khai báo ở file control_time.h
extern DateTime guestStartTime;    // Lấy biến mốc Bắt đầu từ file của bạn
extern DateTime guestEndTime;      // Lấy biến mốc Kết thúc từ file của bạn

HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);
Preferences prefs;

//  PHÂN VÙNG ID
//  Admin  :  1 –  10  (đã lưu sẵn, không đăng ký qua UI)
//  Staff  : 11 –  60  (đăng ký qua UI, có ca làm việc)
//  Client : 61 – 127  (đăng ký qua UI, thời gian tuỳ chỉnh theo 16 số)
#define ADMIN_ID_MIN    1
#define ADMIN_ID_MAX    10
#define STAFF_ID_MIN    11
#define STAFF_ID_MAX    60
#define CLIENT_ID_MIN   61
#define CLIENT_ID_MAX   127

//  KIỂM TRA LOẠI ID
bool isAdminID (uint8_t id) { return id >= ADMIN_ID_MIN  && id <= ADMIN_ID_MAX;  }
bool isStaffID (uint8_t id) { return id >= STAFF_ID_MIN  && id <= STAFF_ID_MAX;  }
bool isClientID(uint8_t id) { return id >= CLIENT_ID_MIN && id <= CLIENT_ID_MAX; }

//  KHỞI TẠO CẢM BIẾN
void initFingerprint() {
  fpSerial.begin(57600, SERIAL_8N1, 14, 15);
  finger.begin(57600);
}
// Đã xóa initRTC() của bạn kia đi để tránh xung đột với file của bạn

//  SCAN VÂN TAY – non-blocking
int scanFingerprintID() {
  int p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) return 0;
  if (p != FINGERPRINT_OK)       return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) return (int)finger.fingerID;

  return -1;
}

//  ĐĂNG KÝ VÂN TAY – blocking
bool enrollFingerprint(uint8_t id, void (*progressCb)(uint8_t) = nullptr) {
  int p = -1;

  // --- LẦN 1: Chụp ảnh + kiểm tra chất lượng ---
  if (progressCb) progressCb(1);
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) { delay(100); continue; }
    if (p != FINGERPRINT_OK) return false;

    // Kiểm tra chất lượng ảnh lần 1 ngay sau getImage
    p = finger.image2Tz(1);
    if (p == FINGERPRINT_OK) break;          // Ảnh tốt -> tiếp tục
    if (p == FINGERPRINT_IMAGEMESS || p == FINGERPRINT_FEATUREFAIL || p == FINGERPRINT_INVALIDIMAGE) {      
      Serial.println("[Enroll] Anh lan 1 kem chat luong, thu lai...");
      delay(200);
      continue;
    }
    return false; // Lỗi khác -> thoát
  }

  // --- NHẤC TAY: Đảm bảo nhấc tay hoàn toàn ---
  if (progressCb) progressCb(2);
  delay(1000);
  // Chờ đến khi THỰC SỰ không còn ngón tay (5 lần liên tiếp)
  int clearCount = 0;
  while (clearCount < 5) {
    if (finger.getImage() == FINGERPRINT_NOFINGER)
      clearCount++;
    else
      clearCount = 0;
    delay(100);
  }

  // --- LẦN 2 + RETRY ---
  bool modelCreated = false;

  for (int retry = 1; retry <= 3; retry++) {
    if (progressCb) progressCb(3);

    // Chụp ảnh lần 2 + kiểm tra chất lượng
    bool gotGoodImage = false;
    while (!gotGoodImage) {
      p = finger.getImage();
      if (p == FINGERPRINT_NOFINGER) { delay(100); continue; }
      if (p != FINGERPRINT_OK) return false;

      p = finger.image2Tz(2);
      if (p == FINGERPRINT_OK) { gotGoodImage = true; break; }
      if (p == FINGERPRINT_IMAGEMESS || p == FINGERPRINT_FEATUREFAIL || p == FINGERPRINT_INVALIDIMAGE) {      // Ảnh lần 2 mờ -> chụp lại
        Serial.println("[Enroll] Anh lan 2 kem chat luong, thu lai...");
        delay(200);
        continue;
      }
      return false;
    }

    // So khớp 2 mẫu
    if (finger.createModel() == FINGERPRINT_OK) {
      modelCreated = true;
      break;
    }

    // --- KHÔNG KHỚP ---
    Serial.printf("[Enroll] Khong khop, lan thu %d/3\n", retry);
    if (retry < 3) {
      if (progressCb) progressCb(5);
      delay(1500);
      if (progressCb) progressCb(2);

      // Chờ nhấc tay hoàn toàn trước khi thử lại
      clearCount = 0;
      while (clearCount < 5) {
        if (finger.getImage() == FINGERPRINT_NOFINGER)
          clearCount++;
        else
          clearCount = 0;
        delay(100);
      }
    }
  }

  if (!modelCreated) return false;

  // --- LƯU ---
  if (progressCb) progressCb(4);
  if (finger.storeModel(id) != FINGERPRINT_OK) return false;

  // --- LƯU TIME ---
  if (isClientID(id)) {
    prefs.begin("clients", false);

    char keyS[8], keyE[8];
    sprintf(keyS, "cs%d", id);
    sprintf(keyE, "ce%d", id);

    prefs.putULong(keyS, guestStartTime.unixtime());
    prefs.putULong(keyE, guestEndTime.unixtime());

    prefs.end();
  }

  return true;
}

//  XÓA VÂN TAY THEO ID
bool deleteFingerprint(uint8_t id) {
  if (id < 1 || id > 127) return false;

  // Kiểm tra ID có tồn tại không trước khi xóa
  if (finger.loadModel(id) != FINGERPRINT_OK) {
    Serial.printf("[Delete] ID %d khong ton tai!\n", id);
    return false;
  }

  bool ok = (finger.deleteModel(id) == FINGERPRINT_OK);
  if (!ok) return false;  // Xóa thất bại thì dừng luôn

  // Xóa dữ liệu NVS kèm theo
  if (isClientID(id)) {
    prefs.begin("clients", false);
    char keyS[8], keyE[8]; 
    sprintf(keyS, "cs%d", id);
    sprintf(keyE, "ce%d", id);
    prefs.remove(keyS);
    prefs.remove(keyE);
    prefs.end();
  }

  if (isStaffID(id)) {
    prefs.begin("shifts", false);
    char key[8]; sprintf(key, "s%d", id);
    prefs.remove(key);
    prefs.end();
  }

  return ok;
}

//  LẤY ID TRỐNG TIẾP THEO
int getNextAvailableID(char type) {
  uint8_t minID = (type == 'S') ? STAFF_ID_MIN  : CLIENT_ID_MIN;
  uint8_t maxID = (type == 'S') ? STAFF_ID_MAX  : CLIENT_ID_MAX;

  for (uint8_t id = minID; id <= maxID; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) return (int)id;
  }
  return -1;
}

//  CÀI ĐẶT CA LÀM VIỆC CHO STAFF
void setStaffShift(uint8_t id, uint8_t startH, uint8_t endH) {
  if (!isStaffID(id)) return;
  prefs.begin("shifts", false);
  char key[8]; sprintf(key, "s%d", id);
  prefs.putUShort(key, ((uint16_t)startH << 8) | endH);
  prefs.end();
}

//  LẤY CA LÀM VIỆC CỦA STAFF
bool getStaffShift(uint8_t id, uint8_t &startH, uint8_t &endH) {
  if (!isStaffID(id)) return false;
  prefs.begin("shifts", true);
  char key[8]; sprintf(key, "s%d", id);
  if (!prefs.isKey(key)) { prefs.end(); return false; }
  uint16_t val = prefs.getUShort(key, 0);
  prefs.end();
  startH = (val >> 8) & 0xFF;
  endH   =  val       & 0xFF;
  return true;
}

// ========================================================
// ĐÃ SỬA: KIỂM TRA HẾT HẠN DỰA TRÊN END_TIME CỦA BẠN
// ========================================================
void checkExpiredClients() {
  prefs.begin("clients", false);
  unsigned long now = rtc.now().unixtime();

  for (uint8_t id = CLIENT_ID_MIN; id <= CLIENT_ID_MAX; id++) {
    char keyE[8]; sprintf(keyE, "ce%d", id); // Lấy khóa End Time ra check
    
    if (prefs.isKey(keyE)) {
      unsigned long endTime = prefs.getULong(keyE, 0);
      
      // Nếu Thời gian hiện tại VƯỢT QUÁ End Time do bạn nhập -> Trảm!
      if (now > endTime) {
        finger.deleteModel(id);
        prefs.remove(keyE);
        
        char keyS[8]; sprintf(keyS, "cs%d", id);
        prefs.remove(keyS);
      }
    }
  }
  prefs.end();
}

//  CHUỖI MÔ TẢ LOẠI ID
const char* getIDTypeName(uint8_t id) {
  if (isAdminID (id)) return "ADMIN";
  if (isStaffID (id)) return "STAFF";
  if (isClientID(id)) return "CLIENT";
  return "UNKNOWN";
}

#endif