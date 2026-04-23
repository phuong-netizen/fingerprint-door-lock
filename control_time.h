
#ifndef CONTROL_TIME_H
#define CONTROL_TIME_H
#include <RTClib.h>

RTC_DS3231 rtc; 

// 1. HÀM KHỞI ĐỘNG
void init_time() {
  if (!rtc.begin()) {
    Serial.println("Loi: Khong tim thay module DS3231!");
  }

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void update_time_LCD() {
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();
    
    DateTime now = rtc.now();
    
    char timeBuffer[17]; 
    
    sprintf(timeBuffer, "%02d/%02d/%02d %02d:%02d", now.day(), now.month(), now.year() % 100, now.hour(), now.minute());

    lcd.setCursor(1, 0); 
    lcd.print(timeBuffer);
  }

}


DateTime guestStartTime; // Biến lưu mốc Bắt Đầu
DateTime guestEndTime;   // Biến lưu mốc Kết Thúc
bool isGuestActive = false;


void setGuestTimeRange(int sd, int sm, int sh, int smin, int ed, int em, int eh, int emin) {
  
  DateTime now = rtc.now(); 
  int currentYear = now.year(); 

  guestStartTime = DateTime(currentYear, sm, sd, sh, smin, 0);
  guestEndTime = DateTime(currentYear, em, ed, eh, emin, 0);
  
  isGuestActive = true; 
}
#endif