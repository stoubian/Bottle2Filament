#include "Adafruit_PCD8544.h"
#include "EncButton2.h"
#include "GyverPID.h"
#include <Arduino.h>

enum Direction { left, right, untouched };

float kp = 20;
float ki = 0.2;
float kd = 1;
int16_t dt = 100;

int t2;
int t3;
int t4;
int t5;
int t6;
int t7;
int t8;
int t9;
int t10;
int steinhartInt;
int targetTemp;

uint8_t attachPCINT(uint8_t);
GyverPID regulator(kp, ki, kd, dt); // coefficient P, coefficient Et,
                                    // coefficient. D, sampling period dt (ms)

bool backlight;
int LCD_SCLK = 9;
int LCD_DIN = 8;
int LCD_DC = 5;
int LCD_CS = 2;
int LCD_RST = 3;
Adafruit_PCD8544 display =
    Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);

int CP_SW = 6;
int CP_CLK = 4;
int CP_DT = 7;
int mode = 0;
EncButton2<EB_ENCBTN, EB_TICK> enc1(INPUT_PULLUP, CP_CLK, CP_DT, CP_SW);

auto Temperature_calculateSteinhart() -> int {
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
void Temperature_changeTargetTemp(Direction ControlPanel_encDirection) {
  if (ControlPanel_encDirection == Direction::right && targetTemp < 300) {
    targetTemp += 5;
  }
  if (ControlPanel_encDirection == Direction::left && targetTemp > 20) {
    targetTemp -= 5;
  }
}
void Temperature_tempLoop() {
  int output = 0; // controller output to controlling device (e.g. PWM value
                  // or servo rotation angle)

  regulator.input = steinhartInt;
  regulator.setpoint = targetTemp;
  regulator.getResultTimer();
  output = regulator.output;
  analogWrite(11, output);
}
void Temperature_setup() {
  targetTemp = 20;

  attachPCINT(CP_CLK);
  attachPCINT(CP_DT);

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
void DisplayScreen_setup(int contrast) {
  display.begin();
  display.setContrast(contrast);
  display.display(); // show splashscreen
  backlight = 0;
}
void DisplayScreen_toggleBacklight(Direction direction) {
  if (direction == Direction::left && !backlight) {
    backlight = 1;
    digitalWrite(12, HIGH);
  }
  if (direction == Direction::right && backlight) {
    backlight = 0;
    digitalWrite(12, LOW);
  }
}
void DisplayScreen_displayOnScreenTxt(char *txt) {
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
void DisplayScreen_displayInfo(int steinhartInt, int targetTemp, int mode,
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
void DisplayScreen_clearDisplay() { display.clearDisplay(); }

auto ControlPanel_encDirection() -> Direction {
  if (enc1.right())
    return Direction::right;
  if (enc1.left())
    return Direction::left;
  return Direction::untouched;
}

void ControlPanel_navigateInMenu() {
  if (mode == 0) {
    Temperature_changeTargetTemp(ControlPanel_encDirection());
  }

  if (mode == 1)
    Serial.println("changeTargetSpeed");
  // changeTargetSpeed(ControlPanel_encDirection());

  if (mode == 2) {
    DisplayScreen_toggleBacklight(ControlPanel_encDirection());
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
      DisplayScreen_displayOnScreenTxt("MOTOR");
    }
    if (mode == 4) {
      mode = 2;
      DisplayScreen_displayOnScreenTxt("LIGHT");
    }
    if (mode == 5) {
      mode = 0;
      DisplayScreen_displayOnScreenTxt("TEMP");
    }
  }
}

uint8_t time, dt1, dt2;

void setup() {
  Serial.begin(115200);

  DisplayScreen_setup(60);
  Temperature_setup();
  enc1.setEncType(0);

  pinMode(A0, INPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // stepper.begin(RPM, MICROSTEPS);

  delay(2000);
  DisplayScreen_clearDisplay(); // clears the screen and buffer
}

void loop() {
  time = millis();
  int steinhartInt = Temperature_calculateSteinhart();
  int regulatorOutput = regulator.output;

  Temperature_tempLoop();

  if ((time - dt1 > 500) && (time - dt2 >= 1000)) {
    dt1 = time;

    Serial.print(steinhartInt);
    Serial.print(",");
    Serial.print(targetTemp);
    Serial.print(",");
    Serial.println(regulatorOutput);
  }

  enc1.tick();
  ControlPanel_navigateInMenu();

  if (enc1.turn())
    dt2 = time;

  DisplayScreen_displayInfo(steinhartInt, targetTemp, mode, 3, regulatorOutput);

  // motorControl();

  // delay(100);
}

// PCINT vectors, you need to throw ticks here
// not necessarily to all vectors, enough to the one involved
// pins 0-7: PCINT2
// pins 8-13: PCINT0
// pins A0-A5: PCINT1
ISR(PCINT0_vect) {
  Serial.println("PCINT0_vect");
  enc1.tick();
}

ISR(PCINT1_vect) {
  Serial.println("PCINT1_vect");
  enc1.tick();
}

ISR(PCINT2_vect) {
  Serial.println("PCINT2_vect");
  enc1.tick();
}

// function to configure PCINT for ATmega328 (UNO, Nano, Pro Mini)
uint8_t attachPCINT(uint8_t pin) { // NOLINT(misc-unused-parameters)
  if (pin < 8) {                   // D0-D7 // PCINT2
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
