#ifndef ADD_H
#define ADD_H

#include <Adafruit_Fingerprint.h>
#include <Preferences.h>
#include <RTClib.h> 


extern RTC_DS3231 rtc;             
extern DateTime guestStartTime;   
extern DateTime guestEndTime;     

HardwareSerial fpSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);
Preferences prefs;

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

void initFingerprint() {
  fpSerial.begin(57600, SERIAL_8N1, 14, 15);
  finger.begin(57600);
}

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

bool enrollFingerprint(uint8_t id, void (*progressCb)(uint8_t) = nullptr) {
  int p = -1;

  // --- LẦN 1 ---
  if (progressCb) progressCb(1);
  while (true) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) break;
    if (p == FINGERPRINT_NOFINGER) { delay(100); continue; }
    return false;
  }
  if (finger.image2Tz(1) != FINGERPRINT_OK) return false;

  // --- NHẤC TAY ---
  if (progressCb) progressCb(2);
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER) delay(100);

  // --- LẦN 2 + RETRY ---
  bool modelCreated = false;

  for (int retry = 1; retry <= 3; retry++) {
    if (progressCb) progressCb(3); // đặt lại vân tay

    while (true) {
      p = finger.getImage();
      if (p == FINGERPRINT_OK) break;
      if (p == FINGERPRINT_NOFINGER) { delay(100); continue; }
      return false;
    }

    if (finger.image2Tz(2) == FINGERPRINT_OK &&
        finger.createModel() == FINGERPRINT_OK) {
      modelCreated = true;
      break;
    }

    // --- KHÔNG KHỚP ---
    if (retry < 3) {
      if (progressCb) progressCb(5); // báo không khớp

      delay(2000);

      if (progressCb) progressCb(2); // nhắc nhấc tay

      while (finger.getImage() != FINGERPRINT_NOFINGER) delay(100);
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
  bool ok = (finger.deleteModel(id) == FINGERPRINT_OK);

  if (isClientID(id)) {
    prefs.begin("clients", false);
    char keyS[8], keyE[8]; 
    sprintf(keyS, "cs%d", id);
    sprintf(keyE, "ce%d", id);
    prefs.remove(keyS); // Xóa Start Time
    prefs.remove(keyE); // Xóa End Time
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


void checkExpiredClients() {
  prefs.begin("clients", false);
  unsigned long now = rtc.now().unixtime();

  for (uint8_t id = CLIENT_ID_MIN; id <= CLIENT_ID_MAX; id++) {
    char keyE[8]; sprintf(keyE, "ce%d", id); // Lấy khóa End Time ra check
    
    if (prefs.isKey(keyE)) {
      unsigned long endTime = prefs.getULong(keyE, 0);
      
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