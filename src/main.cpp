#include <Arduino.h>
#include <Wire.h>
#define MPU_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_GYRO_BLOCK_ADDR 0x3B // and the 14 following addresses (first 0-5 for accel, next 8-13 for gyro, ignore 6-7 its for temp)
#define REG_ACCEL_GYRO_BLOCK_COUNT 14

unsigned long timeCurrent = 0;
unsigned long timeLast = 0;
float gyroAngle = 0.0;
float gyroBiasY = 0.0;
float angle = 0.0;

const unsigned long TIME_INTERVAL = 20;
const float ALPHA = 0.98;

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

  delay(100);
  const int SAMPLES = 2000;
  int32_t gyroSum = 0;
  uint8_t buf[14];

  Serial.println("Accumulating gyro Y axis, please do not touch the robot");

  int collected = 0;
  while (collected < SAMPLES)
  {
    if (readRegBlock(buf, REG_ACCEL_GYRO_BLOCK_ADDR, REG_ACCEL_GYRO_BLOCK_COUNT))
    {
      int16_t rawGyroY = (buf[10] << 8) | buf[11];
      gyroSum += rawGyroY;
      collected++;
    }
  }

  Serial.println("Finished accumulating gyro Y axis");

  gyroBiasY = gyroSum / (float)SAMPLES;

  readRegBlock(buf, REG_ACCEL_GYRO_BLOCK_ADDR, REG_ACCEL_GYRO_BLOCK_COUNT);

  int16_t rawX = (buf[0] << 8) | buf[1];
  int16_t rawZ = (buf[4] << 8) | buf[5];
  angle = degrees(atan2(rawX / 16384.0, rawZ / 16384.0));

  timeLast = millis();
}

void loop()
{
  timeCurrent = millis();

  if (timeCurrent - timeLast >= TIME_INTERVAL)
  {
    unsigned long elapsed = timeCurrent - timeLast;
    timeLast = timeCurrent;
    float dt = elapsed / 1000.0;

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
    float gyroY = -(rawGyroY - gyroBiasY) / 131.0;
    float gyroZ = rawGyroZ / 131.0;

    float accelAngle = degrees(atan2(accelX, accelZ));

    gyroAngle += gyroY * dt;
    angle = ALPHA * (angle + gyroY * dt) + (1.0 - ALPHA) * accelAngle;

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
    Serial.print(gyroZ, 4);
    Serial.print(',');
    Serial.print(accelAngle, 4);
    Serial.print(',');
    Serial.print(gyroAngle, 4);
    Serial.print(',');
    Serial.println(angle, 4);
  }
}
