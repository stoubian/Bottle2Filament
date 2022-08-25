#include "./temperature.h"
#include "../control-panel/control-panel.h"
#include "GyverPID.h"

int Temperature::calculateSteinhart() {
  int t = 0;
  int t1 = 0;

  int SERIAL_R = 4100; // series resistance resistor, 4.1 kΩ

  unsigned long THERMISTOR_R =
      100000;         // nominal resistance of the thermistor, 100 kOhm
  int NOMINAL_T = 25; // nominal temperature (at which TR = 100 kΩ)
  int B = 3950;       // Factor B

  float steinhart = 0;

  t10 = t9;
  t9 = t8;
  t8 = t7;
  t7 = t6;
  t6 = t5;
  t5 = t4;
  t4 = t3;
  t3 = t2;
  t2 = t1;

  t1 = analogRead(A0);

  t = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
  float tr = 10 * 1023.0 / t - 1;
  tr = SERIAL_R / tr;

  steinhart = tr / THERMISTOR_R;           // (R/Ro)
  steinhart = log(steinhart);              // ln(R/Ro)
  steinhart /= B;                          // 1/B * ln(R/Ro)
  steinhart += 1.0 / (NOMINAL_T + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;             // Inverse
  steinhart -= 273.15;

  return int(steinhart);
}

void Temperature::changeTargetTemp(Direction encDirection) {
  if (encDirection == Direction::right && targetTemp < 300) {
    targetTemp += 5;
  }
  if (encDirection == Direction::left && targetTemp > 20) {
    targetTemp -= 5;
  }
}

void Temperature::tempLoop() {
  int output = 0; // controller output to controlling device (e.g. PWM value or
                  // servo rotation angle)

  regulator.input = steinhartInt;
  regulator.setpoint = targetTemp;
  regulator.getResultTimer();
  output = regulator.output;
  analogWrite(11, output);
}

void Temperature::setup() {
  targetTemp = 20;

  attachPCINT(ControlPanel::CLK);
  attachPCINT(ControlPanel::DT);

  regulator.setDirection(
      NORMAL); // direction of regulation (NORMAL/REVERSE). DEFAULT IS NORMAL
  regulator.setLimits(0,
                      255); // limits (set for 8-bit PWM). DEFAULT IS 0 AND 255
  regulator.setpoint =
      50; // tell the controller what temperature it should maintain
  // during work, you can modify the coefficients
  // regulator.Kp = 5.2;
  // regulator.Ki += 0.5;
  // regulator.Kd = 0;
}

// function to configure PCINT for ATmega328 (UNO, Nano, Pro Mini)
uint8_t
Temperature::attachPCINT(uint8_t pin) { // NOLINT(misc-unused-parameters)

  if (pin < 8) { // D0-D7 // PCINT2
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << pin);
    return 2;
  }

  if (pin > 13) { // A0-A5 // PCINT1
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << (pin - 14));
    return 1;
  }

  // D8-D13 // PCINT0
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << (pin - 8));
  return 0;
}