#include <Arduino.h>

#define SIGNAL_PIN 2
#define LED_PIN 9
#define SERIAL_PRINT_INTERVAL 200
#define CYCLES_PER_SIGNAL 500
#define LED_THRESHOLD 700
#define SENSITIVITY 20.0

volatile unsigned long lastSignalTime = 0;
volatile unsigned long signalTimeDelta = 0;
volatile boolean firstSignal = true;
volatile unsigned long storedTimeDelta = 0;
volatile unsigned long cycleCount = 0;
unsigned long lastPrintTime = 0;

void onPulse()
{
  cycleCount++;
  if (cycleCount >= CYCLES_PER_SIGNAL)
  {
    cycleCount = 0;
    unsigned long currentTime = micros();
    signalTimeDelta = currentTime - lastSignalTime;
    lastSignalTime = currentTime;

    if (firstSignal)
    {
      firstSignal = false;
    }
    else if (storedTimeDelta == 0)
    {
      storedTimeDelta = signalTimeDelta;
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Calibrating");

  pinMode(SIGNAL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  attachInterrupt(digitalPinToInterrupt(SIGNAL_PIN), onPulse, RISING);
}

void loop()
{
  int storedTimeDeltaDifference = (storedTimeDelta - signalTimeDelta) * SENSITIVITY;

  digitalWrite(LED_PIN, storedTimeDeltaDifference > LED_THRESHOLD ? HIGH : LOW);

  unsigned long now = millis();
  if (now - lastPrintTime >= SERIAL_PRINT_INTERVAL)
  {
    lastPrintTime = now;
    if (storedTimeDelta == 0)
    {
      Serial.println("Calibrating...");
    }
    else
    {
      Serial.print("Baseline: ");          Serial.print(storedTimeDelta);
      Serial.print(" us  |  Current: ");   Serial.print(signalTimeDelta);
      Serial.print(" us  |  Diff: ");      Serial.print(storedTimeDeltaDifference);
      Serial.print("  |  Sensitivity: ");  Serial.print(SENSITIVITY);
      Serial.print("  |  Metal: ");
      Serial.println(storedTimeDeltaDifference > LED_THRESHOLD ? "YES" : "no");
    }
  }
}
