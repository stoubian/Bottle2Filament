#include "./display-screen.h"

void DisplayScreen::setup(int contrast) {
  display.begin();
  display.setContrast(contrast);
  display.display(); // show splashscreen
  backlight = 0;
}

void DisplayScreen::toggleBacklight(Direction direction) {
  if (direction == Direction::left && !backlight) {
    backlight = 1;
    digitalWrite(12, HIGH);
  }
  if (direction == Direction::right && backlight) {
    backlight = 0;
    digitalWrite(12, LOW);
  }
}

void DisplayScreen::displayOnScreenTxt(char *txt) {
  int x = (display.width() - (strlen(txt) * 12)) / 2;
  int y = (display.height() - 16) / 2;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.setCursor(x, y);
  display.println(txt);
  display.display();
  delay(500);
}

void DisplayScreen::displayInfo(int steinhartInt, int targetTemp, int mode,
                                int motor, int regulatorOutput) {
  display.clearDisplay();
  int row = 0;
  int col = 6;

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, row);
  display.print("SETTINGS");

  row += 8;
  display.setCursor(col, row);
  display.print("T: ");
  display.print(steinhartInt);
  display.print(" C");

  row += 8;
  display.setCursor(col, row);
  display.print("Target: ");
  display.print(targetTemp);
  display.print(" C");

  if (mode == 0) {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  display.setCursor(col, row);
  display.print("Motor: ");
  display.print(motor);

  if (mode == 1) {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  display.setCursor(col, row);
  display.print("Backlight: ");
  display.print(backlight);

  if (mode == 2) {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  col = (display.width() - (7 * 6)) / 2;

  if (regulatorOutput > 0) {
    display.setCursor(col, row);
    display.print("HEATING");
  } else {
    display.setCursor(col, row);
    display.print("READY !");
  }

  display.display();
}

void DisplayScreen::clearDisplay() { display.clearDisplay(); }
