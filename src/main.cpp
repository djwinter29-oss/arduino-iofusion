#include <Arduino.h>

#include "timer.h"
#include "version_info.h"
#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"

namespace {
  constexpr float kTimerTickHz = 1000.0f; // Timer2 tick frequency

  Timer2Driver timer2;
  AnalogSampler analogSampler;
  DigiIn digiIn;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  const uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};
  // pwm 9,10

  void timerTickHandler() {
    analogSampler.onTick();
    digiIn.onTick();
    encoder.onTick();
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  pinMode(LED_BUILTIN, OUTPUT);

  analogSampler.begin(kAnalogPins, static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])));

  digiIn.begin(kDigitalPins, static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])), 500, kTimerTickHz, true);

  encoder.begin(4, 5, 6, 7);

  pwm.set(0, 1000.0f, 50.0f);
  pwm.set(1, 1000.0f, 25.0f);

  timer2.beginHz(kTimerTickHz);
  timer2.attachCallback(timerTickHandler);
}

void loop() {
  analogSampler.sampleIfDue();

  static unsigned long lastPrint = 0;
  const unsigned long now = millis();
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    Serial.println("-- Sensor Snapshot --");

    for (uint8_t i = 0; i < analogSampler.getChannelCount(); ++i) {
      float volts = analogSampler.getValue(i);
      Serial.print("A");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(volts, 3);
      Serial.println(" V");
    }

    for (uint8_t i = 0; i < digiIn.getPinCount(); ++i) {
      Serial.print("D");
      Serial.print(kDigitalPins[i]);
      Serial.print(" freq=");
      Serial.print(digiIn.getFrequency(i), 1);
      Serial.print(" Hz duty=");
      Serial.print(digiIn.getDutyCycle(i), 1);
      Serial.println(" %");
    }

    Serial.print("Encoder position: ");
    Serial.print(encoder.getPosition());
    Serial.print(" dir=");
    Serial.println(encoder.getDirection() ? "CW" : "CCW");
  }
}
