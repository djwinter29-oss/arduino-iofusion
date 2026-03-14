#include "avr_timer2_driver.h"

#if defined(__AVR__)

namespace {

constexpr uint32_t kTimer2CpuHz =
#if defined(F_CPU)
    F_CPU;
#else
    16000000UL;
#endif

uint16_t computeTimer2Ocr(float freqHz, uint16_t& chosenPres) {
  if (freqHz <= 0.0f) return 0;

  const uint16_t presVals[] = {1, 8, 32, 64, 128, 256, 1024};
  for (uint8_t i = 0; i < sizeof(presVals) / sizeof(presVals[0]); ++i) {
    float pres = static_cast<float>(presVals[i]);
    float ocr = (kTimer2CpuHz / (pres * freqHz)) - 1.0f;
    if (ocr >= 1.0f && ocr <= 255.0f) {
      chosenPres = presVals[i];
      return static_cast<uint16_t>(ocr + 0.5f);
    }
  }

  chosenPres = 0;
  return 0;
}

}  // namespace

Timer2Driver* volatile Timer2Driver::_activeDriver = nullptr;

Timer2Driver::Timer2Driver() {
  resetCallbacks();
}

uint16_t Timer2Driver::begin(const Config& config) {
  return beginHz(config.frequencyHz);
}

uint16_t Timer2Driver::beginHz(float freqHz) {
  uint16_t chosenPres = 0;
  uint16_t chosenOCR = computeTimer2Ocr(freqHz, chosenPres);
  if (chosenPres == 0) return 0;

  uint8_t csbits = 0;
  switch (chosenPres) {
    case 1:
      csbits = _BV(CS20);
      break;
    case 8:
      csbits = _BV(CS21);
      break;
    case 32:
      csbits = _BV(CS20) | _BV(CS21);
      break;
    case 64:
      csbits = _BV(CS22);
      break;
    case 128:
      csbits = _BV(CS22) | _BV(CS20);
      break;
    case 256:
      csbits = _BV(CS22) | _BV(CS21);
      break;
    case 1024:
      csbits = _BV(CS22) | _BV(CS21) | _BV(CS20);
      break;
    default:
      csbits = _BV(CS20);
      break;
  }

  noInterrupts();
  if (_activeDriver != nullptr) {
    interrupts();
    return 0;
  }

  resetCallbacks();

  // Stop Timer2 and clear any stale counter/interrupt state before arming it.
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
  TCNT2 = 0;
  TIFR2 = _BV(OCF2A) | _BV(TOV2) | _BV(OCF2B);

  // Configure CTC mode and preload the compare value while the timer is stopped.
  TCCR2A = _BV(WGM21);
  OCR2A = (uint8_t)chosenOCR;
  _activeDriver = this;

  // Enable compare-match dispatch only after the timer state and owner are valid.
  TIMSK2 = _BV(OCIE2A);
  TCCR2B = csbits;
  interrupts();
  return (uint16_t)chosenOCR;
}

void Timer2Driver::stop() {
  noInterrupts();
  TIMSK2 &= ~_BV(OCIE2A);
  TCCR2A = 0;
  TCCR2B = 0;
  if (_activeDriver == this) {
    _activeDriver = nullptr;
  }
  resetCallbacks();
  interrupts();
}

bool Timer2Driver::attachCallback(Timer2Callback cb) {
  if (cb == nullptr) return false;

  noInterrupts();
  if (_activeDriver != this) {
    interrupts();
    return false;
  }

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == cb) {
      interrupts();
      return false;
    }
  }

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == nullptr) {
      _cbs[i] = cb;
      interrupts();
      return true;
    }
  }
  interrupts();
  return false;
}

bool Timer2Driver::detachCallback(Timer2Callback cb) {
  if (cb == nullptr) return false;

  noInterrupts();
  if (_activeDriver != this) {
    interrupts();
    return false;
  }

  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == cb) {
      _cbs[i] = nullptr;
      interrupts();
      return true;
    }
  }
  interrupts();
  return false;
}

// No isSampleDue() flag — use attachCallback() for ISR work.

// ISR for Timer2 Compare Match A
ISR(TIMER2_COMPA_vect) {
  // ISR dispatches callbacks for the active Timer2 owner instance.
  Timer2Driver::handleInterrupt();
}

void Timer2Driver::handleInterrupt() {
  Timer2Driver* driver = _activeDriver;
  if (driver != nullptr) driver->dispatchCallbacks();
}

void Timer2Driver::resetCallbacks() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    _cbs[i] = nullptr;
  }
}

void Timer2Driver::dispatchCallbacks() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    Timer2Callback cb = _cbs[i];
    if (cb) cb();
  }
}
#endif  // __AVR__
