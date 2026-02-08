#include <Arduino.h>


#include "version_info.h"
#include "timer.h"
#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"
#include "cmdline.h"

namespace {
  constexpr float kTimerTickHz = 10000.0f; // Timer2 tick frequency

  Timer2Driver timer2;
  AnalogSampler analogSampler;
  DigiIn digiIn;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  const uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};
  // pwm 9,10

  CmdLine cmdLine(
    analogSampler,
    digiIn,
    encoder,
    pwm,
    kAnalogPins,
    static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
    kDigitalPins,
    static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0]))
  );

  void timerTickHandler() {
    analogSampler.onTick();
    digiIn.onTick();
    encoder.onTick();
  }

  void processSerial() {
    cmdLine.processSerial();
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

  pwm.begin(100.0f);
  pwm.setDuty(0, 50.0f);
  pwm.setDuty(1, 25.0f);

  timer2.beginHz(kTimerTickHz);
  timer2.attachCallback(timerTickHandler);
}

void loop() {
  analogSampler.sampleIfDue();
  digiIn.updateIfReady();
  processSerial();
}
