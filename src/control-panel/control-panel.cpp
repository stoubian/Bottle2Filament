#include "../display-screen/display-screen.h"
#include "../temperature/temperature.h"

#include "./control-panel.h"

Temperature temperature;
DisplayScreen displayScreen;

auto ControlPanel::encDirection() -> Direction {
  if (enc1.right())
    return Direction::right;
  if (enc1.left())
    return Direction::left;
  return Direction::untouched;
}

void ControlPanel::navigateInMenu() {
  if (mode == 0) {
    temperature.changeTargetTemp(encDirection());
  }

  if (mode == 1)
    Serial.println("changeTargetSpeed");
  // changeTargetSpeed(encDirection());

  if (mode == 2) {
    displayScreen.toggleBacklight(encDirection());
  }

  if (enc1.press()) {
    Serial.println("enc PRESS");
    if (mode == 0) {
      mode = 3;
    }
    if (mode == 1) {
      mode = 4;
    }
    if (mode == 2) {
      mode = 5;
    }
  }
  if (enc1.release()) {
    Serial.println("enc RELEASE");
    if (mode == 3) {
      mode = 1;
      displayScreen.displayOnScreenTxt("MOTOR");
    }
    if (mode == 4) {
      mode = 2;
      displayScreen.displayOnScreenTxt("LIGHT");
    }
    if (mode == 5) {
      mode = 0;
      displayScreen.displayOnScreenTxt("TEMP");
    }
  }
}
