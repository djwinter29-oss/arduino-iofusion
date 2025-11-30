#include <Arduino.h>
#include "digital_scanner.h"
#include "AnalogScanner.h"

// Example pins (adjust for your board)
// We'll use DigitalScanner to automatically choose pins on UNO.
DigitalScanner scanner(4, 1.0f); // samples=4, offThresholdHz=1.0



AnalogScanner analogs(6, 8); // A0..A5, 8-sample average

void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("input_measure: DigitalScanner example");

  scanner.autoSelectPinsForUno();
  // Enable software/internal pull-ups for analog pins when you want default-high behavior
  // Set to `true` to enable internal pull-ups, or `false` to leave pins floating (use external pulldowns instead)
  analogs.setUsePullup(false);
  analogs.begin();

  // Register an edge callback (runs inside ISR on AVR; keep it minimal)
  scanner.setEdgeCallback([](uint8_t pin, bool rising){
    // Avoid heavy work in ISR; set a volatile flag or just toggle an LED.
    // Here we simply toggle the built-in LED for demonstration (safe-ish but keep minimal).
    if (rising) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
  });
}

unsigned long lastPrint = 0;

void loop() {
  scanner.update();
  analogs.sampleIfDue();

  unsigned long now = millis();
  if (now - lastPrint >= 500) {
    lastPrint = now;
    Serial.println("--- Scan ---");
    for (uint8_t i = 0; i < scanner.getPinCount(); ++i) {
      Serial.print("Pin "); Serial.print(scanner.getPin(i));
      Serial.print(": freq="); Serial.print(scanner.getFrequencyHz(i), 2);
      Serial.print(" Hz duty="); Serial.print(scanner.getDutyCyclePercent(i), 1);
      Serial.println(" %");
    }
    Serial.print("Analog inputs (pullup="); Serial.print(analogs.getUsePullup() ? "on" : "off"); Serial.println("):");
    for (uint8_t a = 0; a < analogs.getChannelCount(); ++a) {
      int v = analogs.getAverage(a);
      float volts = ((float)v * 5.0f) / 1023.0f;
      Serial.print("A"); Serial.print(a); Serial.print(" avg="); Serial.print(v);
      Serial.print(" ("); Serial.print(volts, 3); Serial.println(" V)");
    }
    
  }
}
