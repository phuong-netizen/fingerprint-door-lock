int clientTimeIndex = 0;
char clientTimeInput[17];
byte deleteIndex = 0;
char deleteInput[4]; 
const int cursorCols[8] = {0, 2, 4, 6, 9, 11, 13, 15};

void handleInit(char key) {
  if(key == '1') {
    currentScreen = SCREEN_PASS;
    lcd_OPEN();
  }
}

void handlePass(char key) {

  // Nếu nhấn số
  if(key >= '0' && key <= '9') {

    if(passIndex < PASS_LENGTH) {

      inputPass[passIndex] = key;

      lcd.setCursor(4 + passIndex*2, 1);
      lcd.print("*");

      passIndex++;
    }

    // Khi nhập đủ 4 số
    if(passIndex == PASS_LENGTH) {

      inputPass[PASS_LENGTH] = '\0';

      delay(300);  // cho đẹp

      if(strcmp(inputPass, correctPass) == 0) {

        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("ACCESS OK");

        currentScreen = SCREEN_MENU;
        lcd_menu();
      } else {

        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("WRONG PASS");
        delay(2000);
        currentScreen = SCREEN_PASS;
        lcd_OPEN();
      }

      passIndex = 0;
      memset(inputPass, 0, sizeof(inputPass));
      
    }
  }

  // Nhấn * để back
  if(key == '*') {
    passIndex = 0;
    memset(inputPass, 0, sizeof(inputPass));
    currentScreen = SCREEN_INIT;
    lcd_init();
  }
}

void handleMenu(char key) {

  if(key == '1') {
    currentScreen = SCREEN_PIN;
    lcd_PIN();
  }

  else if(key == '2') {
    currentScreen = SCREEN_MANAGE;
    lcd_MANAGE();
  }

  else if(key == '*') {
    currentScreen = SCREEN_INIT;
    lcd_init();
  }
}

void handlePIN(char key) {

  if(key == '1') {
    currentScreen = SCREEN_OPEN;
  }

  else if(key == '2') {
    currentScreen = SCREEN_CHANGE;
    lcd_CHANGE();
  }

  else if(key == '*') {
    currentScreen = SCREEN_MENU;
    lcd_menu();
  }
}

void handleManage(char key) {

  if(key == '1') {
    currentScreen = SCREEN_ADD;
    lcd_ADD();
  }

  else if(key == '2') {
    currentScreen = SCREEN_DELETE;
    lcd_DELETE();
    // bạn có thể làm lcd_DELETE()
  }

  else if(key == '*') {
    currentScreen = SCREEN_MENU;
    lcd_menu();
  }
}

void handleAdd(char key) {
  if(key == '1') {
    currentScreen = SCREEN_CLIENT;
    lcd_CLIENT();
  }

  if(key == '2') {
    currentScreen = SCREEN_STAFF;
    lcd_STAFF();
  }

  if(key == '*') {
    currentScreen = SCREEN_MANAGE;
    lcd_MANAGE();
  }
}



void handleChange(char key) {
  // Nếu nhấn số
  if(key >= '0' && key <= '9') {
    if(passIndex < PASS_LENGTH) {
      inputPass[passIndex] = key;
      lcd.setCursor(4 + passIndex*2, 1);
      lcd.print("*");
      passIndex++;
    }

    // Khi nhập đủ 4 số
    if(passIndex == PASS_LENGTH) {
      inputPass[PASS_LENGTH] = '\0';
      delay(300);  // cho đẹp

      if(strcmp(inputPass, correctPass) == 0) {
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("CORRECT PASS");
        delay(1000);
        
        // Reset bộ đệm TRƯỚC khi sang màn hình mới
        passIndex = 0;
        memset(inputPass, 0, sizeof(inputPass));
        
        // Chuyển sang màn hình nhập pass mới
        currentScreen = SCREEN_NEWPASS;
        lcd_NEWPASS();
        
      } else {
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("WRONG PASS");
        delay(2000);
        
        passIndex = 0;
        memset(inputPass, 0, sizeof(inputPass));
        currentScreen = SCREEN_CHANGE;
        lcd_CHANGE();
      }
    }
  }
  // Nhấn * để back
  else if(key == '*') {
    passIndex = 0;
    memset(inputPass, 0, sizeof(inputPass));
    currentScreen = SCREEN_PIN;
    lcd_PIN();
  }
}



void handleOpen(char key) {
        lcd.clear();
        lcd.setCursor(3,0);
        lcd.print("OPENED");

        relay_set(RELAY_ON);
        delay(3000);
        relay_set(RELAY_OFF);
        currentScreen = SCREEN_INIT;
        lcd_init();
  }

void handleNewPass(char key) {
  // Nếu nhấn số
  if(key >= '0' && key <= '9') {
    if(passIndex < PASS_LENGTH) {
      inputPass[passIndex] = key;
      lcd.setCursor(4 + passIndex*2, 1);
      lcd.print("*"); // In dấu * thay cho số
      passIndex++;
    }

    // Khi nhập đủ 4 số
    if(passIndex == PASS_LENGTH) {
      inputPass[PASS_LENGTH] = '\0';
      delay(300);  // cho đẹp
      
      // LƯU MẬT KHẨU MỚI VÀO correctPass
      strcpy(correctPass, inputPass);
      
      // Báo thành công
      lcd.clear();
      lcd.setCursor(2, 0);
      lcd.print("PASS CHANGED!");
      delay(2000);
      
      // Reset biến và quay về màn hình đầu
      passIndex = 0;
      memset(inputPass, 0, sizeof(inputPass));
      currentScreen = SCREEN_INIT;
      lcd_init();
    }
  }
  // Nhấn * để back lại bước trước
  else if(key == '*') {
    passIndex = 0;
    memset(inputPass, 0, sizeof(inputPass));
    currentScreen = SCREEN_CHANGE;
    lcd_CHANGE();
  }
}

// Hàm này giúp LCD hiển thị trạng thái khi đang quét vân tay
void enrollProgress(uint8_t step) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (step == 1) {
    lcd.print("PLACE FINGER");
  } else if (step == 2) {
    lcd.print("REMOVE FINGER");
  } else if (step == 3) {
    lcd.print("PLACE AGAIN");
  } else if (step == 4) {
    lcd.print("SAVING DATA ...");
  } else if (step == 5) {
    lcd.print("NOT MATCH!");
    lcd.setCursor(0, 1);
    lcd.print("TRY AGAIN...");
  }
}

void handleClient(char key) {
  // 1. NẾU BẤM PHÍM SỐ TỪ 0-9
  if (key >= '0' && key <= '9') {
    clientTimeInput[clientTimeIndex] = key; // Lưu số vừa bấm vào mảng
    
    int pos = clientTimeIndex % 8; 
    lcd.setCursor(cursorCols[pos], 1);
    lcd.print(key);                   
    
    clientTimeIndex++; 

    if (clientTimeIndex == 8) {
      delay(200); 
      
      lcd.clear();
      lcd.setCursor(4, 0); 
      lcd.print("END TIME");                // Đổi tiêu đề dòng trên
      lcd.setCursor(0, 1); 
      lcd.print("_ _/_ _  _ _h_ _");         // Trải lại dàn dấu '_' ở dòng dưới
    }
    
    else if (clientTimeIndex == 16) {
      clientTimeInput[16] = '\0'; // Chốt sổ chuỗi ký tự

      // Tách 8 số đầu (Start Time)
      int sd = (clientTimeInput[0]-'0')*10 + (clientTimeInput[1]-'0');
      int sm = (clientTimeInput[2]-'0')*10 + (clientTimeInput[3]-'0');
      int sh = (clientTimeInput[4]-'0')*10 + (clientTimeInput[5]-'0');
      int smin = (clientTimeInput[6]-'0')*10 + (clientTimeInput[7]-'0');

      // Tách 8 số sau (End Time)
      int ed = (clientTimeInput[8]-'0')*10 + (clientTimeInput[9]-'0');
      int em = (clientTimeInput[10]-'0')*10 + (clientTimeInput[11]-'0');
      int eh = (clientTimeInput[12]-'0')*10 + (clientTimeInput[13]-'0');
      int emin = (clientTimeInput[14]-'0')*10 + (clientTimeInput[15]-'0');

      // Gọi hàm lưu sang file control_time.h 
      // (Bắt buộc phải gọi hàm này trước để hệ thống biết Start/End Time là mấy giờ)
      setGuestTimeRange(sd, sm, sh, smin, ed, em, eh, emin);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("FINGDING ID...");
      delay(1000);

      int newClientID = getNextAvailableID('C'); 

      if (newClientID == -1) {
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("BO NHO DAY!");
        delay(2000);
      } 
      else {

        bool isSuccess = enrollFingerprint(newClientID, enrollProgress);

        lcd.clear();
        if (isSuccess) {
          // Báo thành công rực rỡ thật sự
          lcd.setCursor(2, 0); 
          lcd.print("SAVE SUCCESS");
          lcd.setCursor(0, 1); 
          lcd.print("CLIENT ID: "); 
          lcd.print(newClientID); // In ra ID thực tế vừa được cấp
          sendIDList();
        } else {
          lcd.setCursor(0, 0); 
          lcd.print("FINGER ERROR!");
          lcd.setCursor(0, 1);
          lcd.print("TRY AGAIN");
        }
        delay(3000); 
      }
      
      // Reset mọi thứ và đá về màn hình đồng hồ chờ
      clientTimeIndex = 0;
      memset(clientTimeInput, 0, sizeof(clientTimeInput)); // Xóa sạch mảng
      currentScreen = SCREEN_INIT;
      lcd_init();
    }
  }
  
  // 2. NẾU BẤM PHÍM '*' -> HỦY BỎ VÀ QUAY LẠI MÀN HÌNH TRƯỚC
  else if (key == '*') {
    clientTimeIndex = 0;
    memset(clientTimeInput, 0, sizeof(clientTimeInput)); // Xóa sạch mảng
    currentScreen = SCREEN_ADD;
    lcd_ADD(); // Trở về màn hình hiển thị 1.CLIENT / 2.STAFF
  }
}


void handleStaff(char key) {
  uint8_t startH = 0;
  uint8_t endH = 0;
  bool shiftSelected = false; // Biến cờ hiệu kiểm tra xem có bấm đúng phím 1 hoặc 2 không

  // 1. XỬ LÝ KHI BẤM PHÍM CHỌN CA LÀM VIỆC
  if (key == '1') {
    startH = 6;   // Ca 1: 6h sáng
    endH = 14;    // đến 14h chiều
    shiftSelected = true;
  } 
  else if (key == '2') {
    startH = 14;  // Ca 2: 14h chiều
    endH = 22;    // đến 22h đêm
    shiftSelected = true;
  }
  // 2. NẾU BẤM PHÍM '*' -> HỦY BỎ, QUAY VỀ MENU TRƯỚC
  else if (key == '*') {
    currentScreen = SCREEN_ADD;
    lcd_ADD(); // Trở về màn hình hiển thị 1.CLIENT / 2.STAFF
    return;    // Thoát luôn khỏi hàm
  }

  // 3. NẾU ĐÃ CHỌN CA 1 HOẶC CA 2 THÀNH CÔNG -> TIẾN HÀNH LẤY VÂN TAY
  if (shiftSelected == true) {
    
    // Báo cho Admin biết đang xử lý
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("FINDING ID...");
    delay(1000);

    // Tự động quét xem ID STAFF nào từ 11 - 60 đang trống ('S' = Staff)
    int newStaffID = getNextAvailableID('S'); 

    if (newStaffID == -1) {
      lcd.clear();
      lcd.setCursor(0, 0); 
      lcd.print("BO NHO DAY!");
      delay(2000);
    } 
    else {
      // Chờ nhân viên đặt ngón tay 2 lần (Vẫn xài chung hàm enrollProgress hiển thị LCD)
      bool isSuccess = enrollFingerprint(newStaffID, enrollProgress);

      lcd.clear();
      if (isSuccess) {
        
        // ĐIỂM CỐT LÕI: LƯU CA LÀM VIỆC VÀO BỘ NHỚ NGAY LẬP TỨC
        setStaffShift(newStaffID, startH, endH);

        lcd.setCursor(2, 0); 
        lcd.print("SAVE SUCCESS");
        lcd.setCursor(0, 1); 
        lcd.print("ID STAFF: "); 
        lcd.print(newStaffID); // In ra ID thực tế được cấp (từ 11 - 60)
        sendIDList();
      } else {
        lcd.setCursor(0, 0); 
        lcd.print("LOI VAN TAY!");
        lcd.setCursor(0, 1);
        lcd.print("THU LAI SAU...");
      }
      delay(3000); 
    }
    
    // Hoàn tất mọi thứ -> Đá về màn hình đồng hồ chờ
    currentScreen = SCREEN_INIT;
    lcd_init();
  }
}

void handleDelete(char key) {
  // 1. NHẬP SỐ TỪ 0-9 (Nhập tối đa 3 số vì ID lớn nhất là 127)
  if (key >= '0' && key <= '9') {
    if (deleteIndex < 3) {
      deleteInput[deleteIndex] = key;
      
      // In đè số lên vị trí dấu '_' ở tọa độ cột 7, dòng 1
      lcd.setCursor(5 + deleteIndex*2, 1); 
      lcd.print(key);
      
      deleteIndex++;
    }
  }
  
  // 2. BẤM PHÍM '#' ĐỂ XÁC NHẬN XÓA
  else if (key == '#') {
    if (deleteIndex > 0) { // Đảm bảo đã nhập ít nhất 1 số
      deleteInput[deleteIndex] = '\0'; // Chốt chuỗi
      int idToDelete = atoi(deleteInput); // Đổi chuỗi vừa nhập thành số nguyên

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DELETING...");
      delay(500);

      // Gọi hàm xóa vân tay và các thông tin liên quan (Ca làm / Giờ thuê)
      bool isDeleted = deleteFingerprint(idToDelete);

      lcd.clear();
      if (isDeleted) {
        lcd.setCursor(1, 0);
        lcd.print("DELETE SUCCESS");
        lcd.setCursor(4, 1);
        lcd.print("ID: ");
        lcd.print(idToDelete);
        deleteIDFromFirebase(idToDelete);
        sendIDList();
      } else {
        lcd.setCursor(0, 0);
        lcd.print("DELETE FAILED!");
        lcd.setCursor(0, 1);
        lcd.print("ID INVALID");
      }
      delay(2500);
    }
    
    // Reset và quay về màn hình MANAGE (1.ADD / 2.DELETE)
    deleteIndex = 0;
    memset(deleteInput, 0, sizeof(deleteInput));
    currentScreen = SCREEN_INIT;
    lcd_init();
  }
  
  // 3. BẤM PHÍM '*' ĐỂ HỦY BỎ
  else if (key == '*') {
    deleteIndex = 0;
    memset(deleteInput, 0, sizeof(deleteInput));
    currentScreen = SCREEN_MANAGE;
    lcd_MANAGE(); // Quay về màn hình MANAGE
  }
}