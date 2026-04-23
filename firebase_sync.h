
#ifndef FIREBASE_SYNC_H
#define FIREBASE_SYNC_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  


#define WIFI_SSID     "mkhang"
#define WIFI_PASSWORD "anhkhangdeptraiquadiahihi"
#define FIREBASE_URL  "https://doan1-4673b-default-rtdb.asia-southeast1.firebasedatabase.app/"

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] Dang ket noi");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Da ket noi! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WiFi] Ket noi that bai - tiep tuc khong co WiFi");
  }
}

static bool firebasePatch(const String& path, const String& json) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(FIREBASE_URL) + path;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.sendRequest("PATCH", json);
  http.end();
  return (code == 200);
}

static bool firebasePost(const String& path, const String& json) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = String(FIREBASE_URL) + path;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(json);
  http.end();
  return (code == 200);
}


void sendAccessLog(const char* idType, uint8_t id, const char* action, const char* status) {
  if (WiFi.status() != WL_CONNECTED) return;

  // Lấy thời gian từ RTC
  DateTime now = rtc.now();
  char timeStr[20];
  sprintf(timeStr, "%02d/%02d/%02d %02d:%02d:%02d",
    now.day(), now.month(), now.year() % 100,
    now.hour(), now.minute(), now.second());

  // Tạo JSON payload
  StaticJsonDocument<256> doc;
  doc["time"]   = timeStr;
  doc["type"]   = idType;
  doc["id"]     = id;
  doc["action"] = action;
  doc["status"] = status;

  String json;
  serializeJson(doc, json);

  // POST tạo bản ghi mới (Firebase tự sinh key như -Nxxx...)
  firebasePost("/logs.json", json);

  Serial.printf("[Firebase] Log gui: %s #%d -> %s (%s)\n", idType, id, action, status);
}


void sendSystemStatus() {
  if (WiFi.status() != WL_CONNECTED) return;

  DateTime now = rtc.now();
  float rtcVolt = 3.0;

  char timeStr[20]; 
  sprintf(timeStr, "%02d/%02d/%02d %02d:%02d",
    now.day(), now.month(), now.year() % 100, now.hour(), now.minute());

  StaticJsonDocument<128> doc;
  doc["doorOpen"]          = false;
  doc["rtcBatteryVoltage"] = rtcVolt;
  doc["lastSeen"]          = timeStr;

  String json;
  serializeJson(doc, json);
  firebasePatch("/status.json", json);

  Serial.println("[Firebase] Status gui: " + json);
}


void sendDoorEvent(bool isOpen) {
  if (WiFi.status() != WL_CONNECTED) return;

  StaticJsonDocument<64> doc;
  doc["doorOpen"] = isOpen;

  String json;
  serializeJson(doc, json);
  firebasePatch("/status.json", json);
}

void sendIDList() {
  if (WiFi.status() != WL_CONNECTED) return;

  DateTime now = rtc.now();
  String json = "{";

  // ADMIN
  for (uint8_t id = ADMIN_ID_MIN; id <= ADMIN_ID_MAX; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      json += "\"" + String(id) + "\":{\"type\":\"ADMIN\",\"id\":" + id + "},";
    }
  }

  // STAFF
  prefs.begin("shifts", true);
  for (uint8_t id = STAFF_ID_MIN; id <= STAFF_ID_MAX; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      char key[8]; sprintf(key, "s%d", id);
      uint8_t startH = 0, endH = 0;
      if (prefs.isKey(key)) {
        uint16_t val = prefs.getUShort(key, 0);
        startH = (val >> 8) & 0xFF;
        endH   =  val       & 0xFF;
      }
      json += "\"" + String(id) + "\":{\"type\":\"STAFF\",\"id\":" + id
            + ",\"shiftStart\":" + startH + ",\"shiftEnd\":" + endH + "},";
    }
  }
  prefs.end();

  // CLIENT
  prefs.begin("clients", true);
  for (uint8_t id = CLIENT_ID_MIN; id <= CLIENT_ID_MAX; id++) {
    if (finger.loadModel(id) == FINGERPRINT_OK) {
      char keyS[8], keyE[8];
      sprintf(keyS, "cs%d", id);
      sprintf(keyE, "ce%d", id);
      unsigned long startT = prefs.getULong(keyS, 0);
      unsigned long endT   = prefs.getULong(keyE, 0);
      bool expired = (now.unixtime() > endT);
      json += "\"" + String(id) + "\":{\"type\":\"CLIENT\",\"id\":" + id
            + ",\"start\":" + startT + ",\"end\":" + endT
            + ",\"expired\":" + (expired ? "true" : "false") + "},";
    }
  }
  prefs.end();

  if (json.endsWith(",")) json.remove(json.length() - 1);
  json += "}";

  firebasePatch("/ids.json", json);
  Serial.println("[Firebase] ID list da gui len");
}

void deleteIDFromFirebase(uint8_t id) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(FIREBASE_URL) + "/ids/" + String(id) + ".json";
  http.begin(url);
  http.sendRequest("DELETE");
  http.end();

  Serial.printf("[Firebase] Da xoa ID %d khoi Firebase\n", id);
}



#endif
