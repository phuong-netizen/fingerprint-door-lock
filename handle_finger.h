    #ifndef HANDLE_FINGERPRINT_H
#define HANDLE_FINGERPRINT_H

#include "ADD.h" 

#define RELAY 13 

void initDoor() {
    pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, LOW);
    Serial.println("Khoi tao Relay Cua thanh cong!");
}

void openDoorRoutine() {
    Serial.println(">>> DONG DIEN KICH RELAY MO CUA <<<");
    digitalWrite(RELAY, HIGH);
    sendDoorEvent(true);
    delay(3000); 
    digitalWrite(RELAY, LOW);
    sendDoorEvent(false);
    Serial.println(">>> DONG DIEN NGAT RELAY, DONG CUA <<<");
}

void handleFingerprintUnlock() {
    

    if (currentScreen != SCREEN_INIT) return; 

    int id = scanFingerprintID(); 

    if (id > 0) { 
        
        Serial.print("Da quet duoc Van tay ID: ");
        Serial.println(id);

        DateTime now = rtc.now();
        uint32_t currentSecs = now.unixtime();

        if (isAdminID(id)) {
            Serial.println("-> Phan quyen: ADMIN. Tien hanh mo cua!");
            lcd.clear(); lcd.print("DOOR OPENED!");
            openDoorRoutine();
            sendAccessLog("ADMIN", id, "Mo cua", "success");
            lcd_init(); 
        }
        
        else if (isStaffID(id)) {
            Serial.println("-> Phan quyen: STAFF.");
            uint8_t startH, endH;
            if (getStaffShift(id, startH, endH)) {
                if (now.hour() >= startH && now.hour() < endH) {
                    Serial.println("Dung ca lam viec -> Mo cua!");
                    lcd.clear(); lcd.print("DOOR OPENED!");
                    openDoorRoutine();
                    sendAccessLog("STAFF", id, "Mo cua", "success");
                } else {
                    Serial.println("Sai ca lam viec -> Tu choi!");
                    lcd.clear(); lcd.print("WRONG SHIFT!");
                    delay(2000);
                    sendAccessLog("STAFF", id, "Sai ca lam viec", "fail");
                }
            }
            lcd_init();
        }
        
        // NẾU LÀ KHÁCH HÀNG (ID 61-127) 
        else if (isClientID(id)) {
            Serial.println("-> Phan quyen: CLIENT.");
            prefs.begin("clients", true); 
            char keyS[8], keyE[8];
            sprintf(keyS, "cs%d", id);
            sprintf(keyE, "ce%d", id);

            if (prefs.isKey(keyS) && prefs.isKey(keyE)) {
                uint32_t startTime = prefs.getULong(keyS, 0);
                uint32_t endTime = prefs.getULong(keyE, 0);
                prefs.end();

                if (currentSecs < startTime) {
                    Serial.println("Chua toi gio check-in -> Tu choi!");
                    lcd.clear(); lcd.print("NOT TIME YET!");
                    
                    delay(2000);
                } else if (currentSecs > endTime) {
                    Serial.println("Da qua han check-out -> Xoa van tay!");
                    lcd.clear(); lcd.print("EXPIRED!");
                    deleteFingerprint(id);
                    deleteIDFromFirebase(id);
                    sendAccessLog("CLIENT", id, "Het han - Da xoa", "fail");
                    delay(2000);
                } else {
                    Serial.println("Hop le -> Mo cua!");
                    lcd.clear(); lcd.print("WELCOME GUEST!");
                    openDoorRoutine();
                    sendAccessLog("CLIENT", id, "Mo cua", "success");
                }
            } else {
                prefs.end();
                Serial.println("Loi doc du lieu NVS Flash!");
                lcd.clear(); lcd.print("LOI DU LIEU!");
                delay(2000);
            }
            lcd_init();
        }
    }
}
#endif