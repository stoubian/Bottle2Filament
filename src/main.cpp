#include <Arduino.h>
#include <Adafruit_PCD8544.h>
#define LCD_SCLK 9
#define LCD_DIN 8
#define LCD_DC 5
#define LCD_CS 2
#define LCD_RST 3
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);

#define PID_INTEGER
#include "GyverPID.h"
// controller values
int setpoint = 0; // set the value the controller should support
int input = 0;    // sensor signal (for example, the temperature we regulate)
int output = 0;   // controller output to controlling device (e.g. PWM value or servo rotation angle)
int pidMin = 0;   // minimum regulator output
int pidMax = 255; // maximum regulator output
// odds
float Kp = 1.0;
float Ki = 1.0;
float Kd = 1.0;
float _dt_s = 0.1; // iteration time in seconds
// auxiliary variables
int prevInput = 0;
float integral = 0.0;
int t, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
#define B 3950              // Factor B
#define SERIAL_R 4100       // series resistance resistor, 4.1 kΩ
#define THERMISTOR_R 100000 // nominal resistance of the thermistor, 100 kOhm
#define NOMINAL_T 25        // nominal temperature (at which TR = 100 kΩ)

GyverPID regulator(20, 0.2, 1, 100); // coefficient P, coefficient Et, coefficient. D, sampling period dt (ms)

#define CLK 4
#define DT 7
#define SW 6
#include "EncButton2.h"
EncButton2<EB_ENCBTN, EB_TICK> enc1(INPUT_PULLUP, CLK, DT, SW);

int targetTemp = 20;
int motor = 0;
unsigned long time, dt1, dt2;
int steinhartInt;
int mode = 0;
boolean backlight = 0;

void changeTargetTemp()
{
  if (enc1.right() && targetTemp < 300)
    targetTemp += 5;
  if (enc1.left() && targetTemp > 20)
    targetTemp -= 5;
}

void changeTargetSpeed()
{
  if (enc1.right() && motor < 255)
    motor++;
  if (enc1.left() && motor > 0)
    motor--;
}

void toggleBacklight()
{
  if (enc1.right() && backlight == 0)
  {
    backlight = 1;
    digitalWrite(12, HIGH);
  }
  if (enc1.left() && backlight == 1)
  {
    backlight = 0;
    digitalWrite(12, LOW);
  }
}

int calculateSteinhart()
{
  float steinhart;

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

void displayOnScreenTxt(char *txt)
{
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

void displayInfo()
{
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

  if (mode == 0)
  {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  display.setCursor(col, row);
  display.print("Motor: ");
  display.print(motor);

  if (mode == 1)
  {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  display.setCursor(col, row);
  display.print("Backlight: ");
  display.print(backlight);

  if (mode == 2)
  {
    display.setCursor(0, row);
    display.print(">");
  }

  row += 8;
  col = (display.width() - (7 * 6)) / 2;

  if (regulator.output > 0)
  {
    display.setCursor(col, row);
    display.print("HEATING");
  }
  else
  {
    display.setCursor(col, row);
    display.print("READY !");
  }

  display.display();
}

void navigateInMenu()
{
  if (mode == 0)
    changeTargetTemp();

  if (mode == 1)
    changeTargetSpeed();

  if (mode == 2)
    toggleBacklight();

  if (enc1.press())
  {
    Serial.println("enc PRESS");
    if (mode == 0)
      mode = 3;
    if (mode == 1)
      mode = 4;
    if (mode == 2)
      mode = 5;
  }
  if (enc1.release())
  {
    Serial.println("enc RELEASE");
    if (mode == 3)
    {
      mode = 1;
      displayOnScreenTxt("MOTOR");
    }
    if (mode == 4)
    {
      mode = 2;
      displayOnScreenTxt("LIGHT");
    }
    if (mode == 5)
    {
      mode = 0;
      displayOnScreenTxt("TEMP");
    }
  }
}

// function to configure PCINT for ATmega328 (UNO, Nano, Pro Mini)
uint8_t attachPCINT(uint8_t pin)
{
  if (pin < 8)
  { // D0-D7 // PCINT2
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << pin);
    return 2;
  }
  else if (pin > 13)
  { // A0-A5 // PCINT1
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << pin - 14);
    return 1;
  }
  else
  { // D8-D13 // PCINT0
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << pin - 8);
    return 0;
  }
}

void setup()
{
  Serial.begin(115200);

  display.begin();
  display.setContrast(60);
  display.display(); // show splashscreen

  regulator.setDirection(NORMAL); // direction of regulation (NORMAL/REVERSE). DEFAULT IS NORMAL
  regulator.setLimits(0, 255);    // limits (set for 8-bit PWM). DEFAULT IS 0 AND 255
  regulator.setpoint = 50;        // tell the controller what temperature it should maintain
  // during work, you can modify the coefficients
  // regulator.Kp = 5.2;
  // regulator.Ki += 0.5;
  // regulator.Kd = 0;

  pinMode(A0, INPUT);
  enc1.setEncType(0);
  attachPCINT(CLK);
  attachPCINT(DT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  delay(2000);
  display.clearDisplay(); // clears the screen and buffer
}

void loop()
{
  time = millis();
  if ((time - dt1 > 500) && (time - dt2 >= 1000))
  {
    steinhartInt = calculateSteinhart();
    dt1 = time;

    Serial.print(steinhartInt);
    Serial.print(",");
    Serial.print(targetTemp);
    Serial.print(",");
    Serial.println(regulator.output);
  }

  regulator.input = steinhartInt;
  regulator.setpoint = targetTemp;
  regulator.getResultTimer();
  output = regulator.output;
  analogWrite(10, motor);
  analogWrite(11, output);
  enc1.tick();

  navigateInMenu();

  if (enc1.turn())
    dt2 = time;

  displayInfo();

  // delay(100);
}

// PCINT vectors, you need to throw ticks here
// not necessarily to all vectors, enough to the one involved
// pins 0-7: PCINT2
// pins 8-13: PCINT0
// pins A0-A5: PCINT1
ISR(PCINT0_vect)
{
  Serial.println("PCINT0_vect");
  enc1.tick();
}

ISR(PCINT1_vect)
{
  Serial.println("PCINT1_vect");
  enc1.tick();
}

ISR(PCINT2_vect)
{
  Serial.println("PCINT2_vect");
  enc1.tick();
}
