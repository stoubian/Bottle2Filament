#include <Arduino.h>

#include "./control-panel/control-panel.h"
#include "./display-screen/display-screen.h"
#include "./temperature/temperature.h"

uint8_t time, dt1, dt2;

void setup() {
  DisplayScreen displayScreen;
  Temperature temperature;

  Serial.begin(115200);

  displayScreen.setup(60);
  temperature.setup();

  pinMode(A0, INPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // stepper.begin(RPM, MICROSTEPS);

  delay(2000);
  displayScreen.clearDisplay(); // clears the screen and buffer
}

void loop() {
  DisplayScreen displayScreen;
  Temperature temperature;
  ControlPanel controlPanel;

  time = millis();
  int steinhartInt = temperature.calculateSteinhart();
  int targetTemp = temperature.targetTemp;
  int regulatorOutput = temperature.regulator.output;

  if ((time - dt1 > 500) && (time - dt2 >= 1000)) {
    dt1 = time;

    Serial.print(steinhartInt);
    Serial.print(",");
    Serial.print(targetTemp);
    Serial.print(",");
    Serial.println(regulatorOutput);
  }

  controlPanel.enc1.tick();
  controlPanel.navigateInMenu();

  if (controlPanel.enc1.turn())
    dt2 = time;

  displayScreen.displayInfo(steinhartInt, targetTemp, controlPanel.mode, 3,
                            regulatorOutput);

  // motorControl();

  // delay(100);
}

ControlPanel controlPanel;
// PCINT vectors, you need to throw ticks here
// not necessarily to all vectors, enough to the one involved
// pins 0-7: PCINT2
// pins 8-13: PCINT0
// pins A0-A5: PCINT1
ISR(PCINT0_vect) {
  Serial.println("PCINT0_vect");
  controlPanel.enc1.tick();
}

ISR(PCINT1_vect) {
  Serial.println("PCINT1_vect");
  controlPanel.enc1.tick();
}

ISR(PCINT2_vect) {
  Serial.println("PCINT2_vect");
  controlPanel.enc1.tick();
}
