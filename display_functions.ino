void lcdUpdateFirstLine(char * lineOne) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lineOne);
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  lastLcdUpdated = millis();
}

void lcdUpdateBothLines(char * lineOne, char * lineTwo){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lineOne);
  lcd.setCursor(0, 1);
  lcd.print(lineTwo);
  lastLcdUpdated = millis();
}
