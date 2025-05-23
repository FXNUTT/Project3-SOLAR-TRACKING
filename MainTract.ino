// Blynk Configuration
#define BLYNK_TEMPLATE_ID "TMPL6KjLcSt8Y"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "NURKUDJeF6yjLYpW1qQAAYoPqM1CVzlr"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <BH1750.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi Credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "KawNetwork";
char pass[] = "Password";

// Timer for Blynk
BlynkTimer timer;

// I2C Multiplexer and Light Sensors
BH1750 lightMeter;
float lux[4];  // Array to store light intensity from sensors

// Servo Configuration
Servo servo1, servo2;
int angle1 = 90, angle2 = 90;  // Initial angles
int differenceX1, differenceX2;
int differenceY1, differenceY2;
int margin = 20;  // Margin for angle adjustment
bool manualControl = false;  // Flag for manual/auto mode

// LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Select I2C channel on TCA9548A
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(0x70);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// LCD Setup
void lcdsetup() {
  tcaSelect(4);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
}

// Display light values on LCD
void lcdout() {
  lcd.setCursor(0, 0);
  lcd.print("1:");
  lcd.print(lux[0]);
  lcd.print(" 2:");
  lcd.print(lux[1]);

  lcd.setCursor(0, 1);
  lcd.print("3:");
  lcd.print(lux[2]);
  lcd.print(" 4:");
  lcd.print(lux[3]);
}

// Servo Setup
void servosetup() {
  servo1.attach(D3);
  servo2.attach(D4);
  servo1.write(angle1);
  servo2.write(angle2);
}

// Adjust servo on X axis based on light difference
void X() {
  differenceX1 = lux[0] - lux[1];
  differenceX2 = lux[2] - lux[3];

  if ((differenceX1 > margin) || (differenceX2 > margin)) {
    if (angle1 < 180) angle1++;
  } else if ((differenceX1 < -margin) || (differenceX2 < -margin)) {
    if (angle1 > 0) angle1--;
  }

  servo1.write(angle1);

  Serial.print("difference X1: ");
  Serial.println(differenceX1);
  Serial.print("difference X2: ");
  Serial.println(differenceX2);
  Serial.print("angle1: ");
  Serial.println(angle1);
}

// Adjust servo on Y axis based on light difference
void Y() {
  differenceY1 = lux[0] - lux[2];
  differenceY2 = lux[1] - lux[3];

  if ((differenceY1 > margin) || (differenceY2 > margin)) {
    if (angle2 < 180) angle2++;
  } else if ((differenceY1 < -margin) || (differenceY2 < -margin)) {
    if (angle2 > 0) angle2--;
  }

  servo2.write(angle2);

  Serial.print("difference Y1: ");
  Serial.println(differenceY1);
  Serial.print("difference Y2: ");
  Serial.println(differenceY2);
  Serial.print("angle2: ");
  Serial.println(angle2);
}

// Wi-Fi and Blynk Setup
void WiFisetup() {
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(10000L, sendlux);  // Every 10 sec
}

// Send data to Blynk
void sendlux() {
  Blynk.virtualWrite(0, lux[0]);
  Blynk.virtualWrite(1, lux[1]);
  Blynk.virtualWrite(2, lux[2]);
  Blynk.virtualWrite(3, lux[3]);
  Blynk.virtualWrite(4, angle1);
  Blynk.virtualWrite(5, angle2);

  if (!manualControl) {
    X();
    Y();
  }
}

// Manual control from Blynk (X-axis)
BLYNK_WRITE(V4) {
  int x = param.asInt();
  angle1 = map(x, -100, 100, 0, 180);
  servo1.write(angle1);
  manualControl = true;

  Serial.print("Joystick X: ");
  Serial.println(x);
}

// Manual control from Blynk (Y-axis)
BLYNK_WRITE(V5) {
  int y = param.asInt();
  angle2 = map(y, -100, 100, 0, 180);
  servo2.write(angle2);
  manualControl = true;

  Serial.print("Joystick Y: ");
  Serial.println(y);
}

void setup() {
  Serial.begin(115200);
  WiFisetup();

  Wire.begin(D2, D1);  // SDA, SCL
  for (uint8_t i = 0; i < 4; i++) {
    tcaSelect(i);
    lightMeter.begin();
  }

  lcdsetup();
  servosetup();
}

void loop() {
  for (uint8_t i = 0; i <= 5; i++) {
    if (i <= 3) {
      tcaSelect(i);
      lux[i] = lightMeter.readLightLevel();
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(lux[i]);
      Serial.println(" lx");
    }
  }

  Serial.println();
  lcdout();

  Blynk.run();
  timer.run();
}
