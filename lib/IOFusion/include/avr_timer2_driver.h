// Simple Timer2 driver for AVR/Arduino
// Provides periodic interrupts on Timer2 and a tiny encoder signal generator.
#ifndef IOFUSION_AVR_TIMER2_DRIVER_H
#define IOFUSION_AVR_TIMER2_DRIVER_H

#include <Arduino.h>

typedef void (*Timer2Callback)();

class Timer2Driver {
 public:
  Timer2Driver();
  // Start timer at approximate frequency in Hz. Returns actual OCR value used.
  uint16_t beginHz(float freqHz);
  // Stop timer and disable interrupt
  void stop();

  // Attach a callback that will be called from the ISR context.
  // Keep the callback short; heavy work should be deferred to loop().
  // Returns false for null callbacks, duplicates, overflow, or inactive drivers.
  bool attachCallback(Timer2Callback cb);
  // Detach a specific callback previously attached. Pass the same function pointer.
  // Returns false if the callback is not currently registered.
  bool detachCallback(Timer2Callback cb);
  // ISR entry point
  static void handleInterrupt();

 private:
  static const uint8_t MAX_CALLBACKS = 4;
  volatile Timer2Callback _cbs[MAX_CALLBACKS];
  static Timer2Driver* volatile _activeDriver;

  void resetCallbacks();
  void dispatchCallbacks();
};

#endif  // IOFUSION_AVR_TIMER2_DRIVER_H
