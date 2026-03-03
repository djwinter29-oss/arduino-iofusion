#include <Arduino.h>

#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"

AnalogSampler analogSampler;
DigiIn digiIn;
EncoderGenerator encoder;
Timer1PWM pwm;

const uint8_t kAnalogPins[] = {0, 1};
const uint8_t kDigitalPins[] = {2, 3};

void setup() {
  Serial.begin(115200);

  analogSampler.begin(kAnalogPins,
                      static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])));
  analogSampler.setVref(5.0f);

  digiIn.begin(kDigitalPins,
               static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])),
               500,
               1000.0f,
               true);

  // pinA=8, pinB=11, up=12, down=13
  encoder.begin(8, 11, 12, 13);

  // Timer1 PWM on Uno pins 9/10
  pwm.begin(1000.0f);
  pwm.setDuty(0, 25.0f);
  pwm.setDuty(1, 75.0f);
}

void loop() {
  // In a real app, call onTick() from a hardware timer ISR and keep ISR work minimal.
  analogSampler.onTick();
  digiIn.onTick();
  encoder.onTick();

  analogSampler.sampleIfDue();
  digiIn.updateIfReady();

  delay(1);
}
