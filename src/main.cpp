#include <Arduino.h>
#define LED 2

void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(115200); // start comms
  Serial.println("running...");
}

void loop()
{
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(200);
}
