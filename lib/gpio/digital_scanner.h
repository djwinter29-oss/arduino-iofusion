#ifndef DIGITAL_SCANNER_H
#define DIGITAL_SCANNER_H

#include <Arduino.h>


class DigitalScanner {
public:
  struct PinState {
    uint8_t pin;
    int lastReading;
    unsigned long lastEdgeTime;
    unsigned long lastRiseTime;
    unsigned long prevRiseTime;
    unsigned long lastFallTime;
    unsigned long sumPeriodMs;
    unsigned long sumHighMs;
    uint8_t periodCount;
    uint8_t highCount;
    float freqHz;
    float dutyPct;
    bool initialized;
  };

  // Construct with an explicit pins array, or call autoSelectPinsForUno()
  DigitalScanner(const uint8_t *pins, uint8_t pinCount, uint8_t samples = 4, float offThresholdHz = 1.0f);
  DigitalScanner(uint8_t samples = 4, float offThresholdHz = 1.0f);
  ~DigitalScanner();
  // Populate pins automatically for Arduino UNO (skips SCL/SDA and hardware PWM pins)
  void autoSelectPinsForUno();
  void begin();
  // Call frequently in loop()
  void update();

  // IRQ/callback support
  // Callback signature: void callback(uint8_t pin, bool rising)
  typedef void (*EdgeCallback)(uint8_t pin, bool rising);
  // Register callbacks for rise/fall events on any scanned pin
  void setEdgeCallback(EdgeCallback cb);

  // Accessors
  uint8_t getPinCount() const;
  uint8_t getPin(uint8_t idx) const;
  float getFrequencyHz(uint8_t idx) const; // 0.0 when considered off
  float getDutyCyclePercent(uint8_t idx) const; // 0..100

  // Low-level access (used by ISR code in the implementation)
  PinState *states() { return _states; }
  uint8_t pinCount() const { return _pinCount; }
  EdgeCallback edgeCallback() const { return _edgeCb; }

private:
  PinState *_states;
  uint8_t _pinCount;
  uint8_t _samples;
  float _offThresholdHz;
  EdgeCallback _edgeCb;
};

#endif // DIGITAL_SCANNER_H