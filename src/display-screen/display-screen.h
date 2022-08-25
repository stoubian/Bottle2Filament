#ifndef B2F_DISPLAY_SCREEN_H // NOLINT(llvm-header-guard)
#define B2F_DISPLAY_SCREEN_H

#include "../enums.h"
#include <Adafruit_PCD8544.h>

class DisplayScreen {

private:
  bool backlight;
  int LCD_SCLK = 9;
  int LCD_DIN = 8;
  int LCD_DC = 5;
  int LCD_CS = 2;
  int LCD_RST = 3;
  bool init;

public:
  Adafruit_PCD8544 display =
      Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);
  void setup(int);
  void toggleBacklight(Direction);
  void displayOnScreenTxt(char *txt);
  void displayInfo(int steinhartInt, int targetTemp, int mode, int motor,
                   int regulatorOutput);
  void clearDisplay();
};

#endif