#include <Arduino.h>
#include <Wire.h>
#define LED 2
#define MPU_ADDR 0x68
#define REG_WHOAMI 0x75

unsigned long timeCurrent = 0;
unsigned long timeLast = 0;
const unsigned long TIME_INTERVAL = 1000;
bool ledState = false;
uint8_t whoAmI = 0;
bool hasPrinted = false;

uint8_t readRegister(uint8_t reg)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 1);
  return Wire.read();
}

void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(115200); // start comms
  Wire.begin();
}

void loop()
{
  if (!hasPrinted)
  {
    delay(3000);
    whoAmI = readRegister(REG_WHOAMI);
    Serial.print("Who am I: 0x");
    Serial.println(whoAmI, HEX);
    hasPrinted = true;
  }

  timeCurrent = millis();

  if (timeCurrent - timeLast >= TIME_INTERVAL)
  {
    timeLast = timeCurrent;
    ledState = !ledState;

    if (ledState)
    {
      digitalWrite(LED, HIGH);
    }
    else
    {
      digitalWrite(LED, LOW);
    }
  }
}
