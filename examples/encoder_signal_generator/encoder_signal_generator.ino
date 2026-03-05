#include <Arduino.h>

#include "encoder.h"
#include "timer.h"

namespace {
constexpr float kTickHz = 5000.0f;

Timer2Driver timer2;
EncoderGenerator encoder;

// Output A/B pins: 8,11
// Control pins: up=12, down=13 (INPUT_PULLUP)
void onTick() {
  encoder.onTick();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  if (!encoder.begin(8, 11, 12, 13)) {
    Serial.println(F("{\"error\":\"encoder init failed\"}"));
    return;
  }

  if (timer2.beginHz(kTickHz) == 0) {
    Serial.println(F("{\"error\":\"timer2 init failed\"}"));
    return;
  }

  timer2.attachCallback(onTick);
  Serial.println(F("encoder_signal_generator ready"));
}

void loop() {
  static unsigned long lastPrintMs = 0;
  unsigned long now = millis();
  if (now - lastPrintMs >= 200) {
    lastPrintMs = now;

    Serial.print(F("{\"direction\":\""));
    Serial.print(encoder.getDirection() ? F("UP") : F("DOWN"));
    Serial.print(F("\",\"position\":"));
    Serial.print(encoder.getPosition());
    Serial.println(F("}"));
  }
}
