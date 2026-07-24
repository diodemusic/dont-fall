#include <Arduino.h>
#define LED 2

unsigned long timeCurrent = 0;
unsigned long timeLast = 0;
const unsigned long TIME_INTERVAL = 1000;
bool ledState = false;

void setup()
{
  pinMode(2, OUTPUT);
  // Serial.begin(115200); // start comms
  Serial.println("running...");
}

void loop()
{
  timeCurrent = millis();

  if (timeCurrent - timeLast >= TIME_INTERVAL)
  {
    timeLast = timeCurrent;
    ledState = !ledState;

    if (ledState)
    {
      digitalWrite(LED, HIGH);
      // Serial.println("blink high");
    }
    else
    {
      digitalWrite(LED, LOW);
      // Serial.println("blink low");
    }
  }
}
