void lcd_init() {
  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("KHOA CUA VAN TAY");
}

void lcd_menu() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("1.PIN");

  lcd.setCursor(0, 1);
  lcd.print("2.MANAGE");
}

void lcd_PIN() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("1.OPEN");

  lcd.setCursor(0, 1);
  lcd.print("2.CHANGE");
}

void lcd_MANAGE() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("1.ADD");

  lcd.setCursor(0, 1);
  lcd.print("2.DELETE");
}

void lcd_ADD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("1.CLIENT");

  lcd.setCursor(0, 1);
  lcd.print("2.STAFF");
}

void lcd_OPEN() {
  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print("ENTER PASS");

  lcd.setCursor(4, 1);
  lcd.print("_ _ _ _");
}

void lcd_CHANGE() {
  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print("ENTER PASS");

  lcd.setCursor(4, 1);
  lcd.print("_ _ _ _");
}

void lcd_NEWPASS() {
  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("NEW PASS");

  lcd.setCursor(4, 1);
  lcd.print("_ _ _ _");
}

void lcd_CLIENT() {
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("START TIME");

  lcd.setCursor(0, 1);
  lcd.print("_ _/_ _  _ _h_ _");
}

void lcd_END_TIME() {
  lcd.clear();

  lcd.setCursor(2, 0);
  lcd.print("END TIME");

  lcd.setCursor(0, 1);
  lcd.print("_ _/_ _  _ _h_ _");
}

void lcd_STAFF() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("1.Shift1(6-14)");

  lcd.setCursor(0, 1);
  lcd.print("2.Shift2(14-22)");
}

void lcd_DELETE() {
  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("ENTER ID");

  lcd.setCursor(5, 1);
  lcd.print("_ _ _");
}
