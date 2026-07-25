#include <Arduino.h>
#include <Wire.h>
#define MPU_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_GYRO_BLOCK_ADDR 0x3B // and the 14 following addresses (first 0-5 for accel, next 8-13 for gyro, ignore 6-7 its for temp)
#define REG_ACCEL_GYRO_BLOCK_COUNT 14

unsigned long timeCurrent = 0;
unsigned long timeLast = 0;
const unsigned long TIME_INTERVAL = 100;

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

bool readRegBlock(uint8_t *buf, uint8_t readReg, int count)
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(readReg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, count);

  if (Wire.available() != count)
  {
    return false;
  }

  for (int i = 0; i < count; i++)
  {
    buf[i] = Wire.read();
  }

  return true;
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

    uint8_t accelGyroBuf[14];

    if (!readRegBlock(accelGyroBuf, REG_ACCEL_GYRO_BLOCK_ADDR, REG_ACCEL_GYRO_BLOCK_COUNT))
    {
      Serial.println("ERROR READING SENSOR");
      return;
    }

    int16_t rawAccelX = (accelGyroBuf[0] << 8) | accelGyroBuf[1];
    int16_t rawAccelY = (accelGyroBuf[2] << 8) | accelGyroBuf[3];
    int16_t rawAccelZ = (accelGyroBuf[4] << 8) | accelGyroBuf[5];
    float accelX = rawAccelX / 16384.0;
    float accelY = rawAccelY / 16384.0;
    float accelZ = rawAccelZ / 16384.0;

    int16_t rawGyroX = (accelGyroBuf[8] << 8) | accelGyroBuf[9];
    int16_t rawGyroY = (accelGyroBuf[10] << 8) | accelGyroBuf[11];
    int16_t rawGyroZ = (accelGyroBuf[12] << 8) | accelGyroBuf[13];
    float gyroX = rawGyroX / 131.0;
    float gyroY = rawGyroY / 131.0;
    float gyroZ = rawGyroZ / 131.0;

    Serial.print(accelX, 4);
    Serial.print(',');
    Serial.print(accelY, 4);
    Serial.print(',');
    Serial.print(accelZ, 4);
    Serial.print(',');
    Serial.print(gyroX, 4);
    Serial.print(',');
    Serial.print(gyroY, 4);
    Serial.print(',');
    Serial.println(gyroZ, 4);
  }
}
