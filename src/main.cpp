#include <Arduino.h>
#include "digital_scanner.h"

// Example pins (adjust for your board)
// We'll use DigitalScanner to automatically choose pins on UNO.
DigitalScanner scanner(4, 1.0f); // samples=4, offThresholdHz=1.0



void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("input_measure: DigitalScanner example");

  scanner.autoSelectPinsForUno();

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
    
  }
}
