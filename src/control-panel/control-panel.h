#ifndef B2F_CONTROL_PANEL_H // NOLINT(llvm-header-guard)
#define B2F_CONTROL_PANEL_H

#include "../enums.h"
#include "EncButton2.h"

class ControlPanel {
private:
  bool init;
  int SW = 6;

public:
  static int CLK;
  static int DT;

  ControlPanel() {
    if (!init) {
      CLK = 4;
      DT = 7;
      EncButton2<EB_ENCBTN, EB_TICK> enc1(INPUT_PULLUP, CLK, DT, SW);
      enc1.setEncType(0);
      init = true;
    }
  }

  int mode = 0;
  EncButton2<EB_ENCBTN, EB_TICK> enc1;
  Direction encDirection();
  void navigateInMenu();
};

#endif