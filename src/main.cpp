#include <Arduino.h>

#include "digital_scanner.h"
#include "AnalogScanner.h"
#include "timer.h"

// Example pins (adjust for your board)
// We'll use DigitalScanner to automatically choose pins on UNO.
DigitalScanner scanner;
AnalogScanner analogs(6, 8); // A0..A5, 8-sample average
Timer2Driver t2;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("input_measure: DigitalScanner example");

  Serial.println("main: after Serial.begin");

  scanner.addPin(2);
  scanner.addPin(4);
  scanner.addPin(7);
  scanner.setSamples(4);
  scanner.setOffThresholdHz(2.0f);
  //scanner.setEdgeCallback(edgeHandler);
  scanner.begin();
  Serial.println("main: after setDebug");
  Serial.println("main: calling scanner.begin()");
  scanner.begin();
  Serial.println("main: returned from scanner.begin()");
  // Enable software/internal pull-ups for analog pins when you want default-high behavior
  // Set to `true` to enable internal pull-ups, or `false` to leave pins floating (use external pulldowns instead)
  analogs.setUsePullup(false);
  Serial.println("main: after analogs.setUsePullup(false)");
  analogs.begin();
  Serial.println("main: returned from analogs.begin()");

  // Start Timer2 at 100 Hz for ADC sampling
  t2.beginHz(100.0f);

  // Example: setup encoder generator outputs on pins 4 and 5 and input up/down on 6/7
  // (up/down pins are used only if you want input counting; they will attach interrupts)
  encoder.begin(4, 5, 6, 7);
  // Encoder needs 4 timer ticks per full quadrature cycle — use 2 Hz * 4 = 8 Hz timer
  t2.beginHz(2.0f * 4.0f);

  // Optional: attach a tiny ISR callback (runs in ISR) - keep it minimal
  t2.attachCallback([](){
    // This runs in ISR; keep it short. Here we toggle the builtin LED quickly.
    // Avoid Serial or heavy operations.
    static bool s = false;
    s = !s;
    digitalWrite(LED_BUILTIN, s ? HIGH : LOW);
  });

  // Register an edge callback (runs inside ISR on AVR; keep it minimal)
  scanner.setEdgeCallback([](uint8_t pin, bool rising){
    // Avoid heavy work in ISR; set a volatile flag or just toggle an LED.
    // Here we simply toggle the built-in LED for demonstration (safe-ish but keep minimal).
    if (rising) digitalWrite(LED_BUILTIN, HIGH);
    else digitalWrite(LED_BUILTIN, LOW);
  });
  // Example: ensure PWM-capable pins are outputs (analogWrite can be used later)
  pinMode(9, OUTPUT);

  // Stop timer
  TCCR1A = 0;
  TCCR1B = 0;

  // Fast PWM, TOP = ICR1, non-inverting on OC1A (pin 9)
  // WGM13:0 = 14 -> WGM13=1 WGM12=1 WGM11=1 WGM10=0
  TCCR1A = _BV(COM1A1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // Prescaler = 1

  // ICR1 = (F_CPU / (prescaler * f)) - 1 = (16e6 / (1 * 40000)) - 1 = 399
  ICR1 = 399;
  OCR1A = 199; // 50% duty (ICR1/2)
}

unsigned long lastPrint = 0;

void loop() {
  scanner.update();
  analogs.sampleIfDue();

  static unsigned long hb = 0;
  if (millis() - hb >= 1000) {
    hb = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  unsigned long now = millis();
  if (now - lastPrint >= 500) {
    lastPrint = now;
    Serial.println("--- Scan ---");
    for (int i = 0; i < scanner.getPinCount(); i++) {
      Serial.print("Pin ");
      Serial.print(scanner.getPin(i));
      Serial.print(": ");
      Serial.print(scanner.getFrequencyHz(i), 1);
      Serial.print(" Hz, ");
      Serial.print(scanner.getDutyCyclePercent(i), 1);
      Serial.println("%");
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
