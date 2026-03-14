#include <Arduino.h>

#include "avr_timer2_driver.h"
#include "digital_input_monitor.h"

namespace {
constexpr float kTickHz = 2000.0f;  // Timer2 ISR frequency
constexpr uint16_t kWindowTicks = 400;

Timer2Driver timer2;
DigitalInputMonitor digitalMonitor;

const uint8_t kInputPins[] = {2, 3};

void onTick() {
  digitalMonitor.onTick();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  bool ok = digitalMonitor.begin(kInputPins, 2, kWindowTicks, kTickHz, true);
  if (!ok) {
    Serial.println(F("{\"error\":\"digital monitor init failed\"}"));
    return;
  }

  if (timer2.beginHz(kTickHz) == 0) {
    Serial.println(F("{\"error\":\"timer2 init failed\"}"));
    return;
  }

  timer2.attachCallback(onTick);
  Serial.println(F("frequency_monitor ready"));
}

void loop() {
  digitalMonitor.updateIfReady();

  Serial.print(F("{\"d2\":{\"freq\":"));
  Serial.print(digitalMonitor.getFrequency(0), 1);
  Serial.print(F(",\"duty\":"));
  Serial.print(digitalMonitor.getDutyCycle(0), 1);
  Serial.print(F("},\"d3\":{\"freq\":"));
  Serial.print(digitalMonitor.getFrequency(1), 1);
  Serial.print(F(",\"duty\":"));
  Serial.print(digitalMonitor.getDutyCycle(1), 1);
  Serial.println(F("}}"));

  delay(250);
}
