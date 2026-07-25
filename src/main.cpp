#include <Arduino.h>
#include <Wire.h>
#define MPU_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B

unsigned long timeCurrent = 0;
unsigned long timeLast = 0;
const unsigned long TIME_INTERVAL = 1000;

uint8_t readRegister(uint8_t reg)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 1);
  return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void wakeMpu()
{
  uint8_t sleepCurrent = readRegister(REG_PWR_MGMT_1);
  sleepCurrent = sleepCurrent & ~(1 << 6);
  writeRegister(REG_PWR_MGMT_1, sleepCurrent);
}

void readAccel(uint8_t buf[6])
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6);

  for (int i = 0; i < 6; i++)
  {
    buf[i] = Wire.read();
  }
}

void setup()
{
  Serial.begin(115200); // start comms
  Wire.begin();
  wakeMpu();
}

void loop()
{
  timeCurrent = millis();

  if (timeCurrent - timeLast >= TIME_INTERVAL)
  {
    timeLast = timeCurrent;
    uint8_t buf[6];
    readAccel(buf);

    int16_t accelX = (buf[0] << 8) | buf[1];
    int16_t accelY = (buf[2] << 8) | buf[3];
    int16_t accelZ = (buf[4] << 8) | buf[5];

    Serial.print("X: ");
    Serial.print(accelX);
    Serial.print('\t');
    Serial.print("Y: ");
    Serial.print(accelY);
    Serial.print('\t');
    Serial.print("Z: ");
    Serial.println(accelZ);
  }
}
