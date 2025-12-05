// Digital input frequency and duty measurement
#ifndef IOFUSION_DIGIIN_H
#define IOFUSION_DIGIIN_H

#include <Arduino.h>

class DigiIn {
public:
	DigiIn();
	// Begin monitoring pins: `pins` array of `count` pin numbers (max 8).
	// `windowTicks` is the number of onTick() samples per measurement window.
	// `tickHz` is the frequency (Hz) at which onTick() will be called.
	bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks = 1000, float tickHz = 1000.0f, bool usePullup = false);

	// Called from ISR (very short). Samples pins and accumulates counts.
	void onTick();

	// Accessors (safe to call from main context)
	uint8_t getPinCount() const;
	float getFrequency(uint8_t idx) const; // Hz
	float getDutyCycle(uint8_t idx) const; // percent 0..100

private:
	static const uint8_t MAX_PINS = 8;
	uint8_t _pins[MAX_PINS];
	uint8_t _pinCount = 0;
	uint16_t _windowTicks = 1000;
	float _tickHz = 1000.0f;

	// ISR-updated counters
	volatile uint16_t _samplesInWindow = 0;
	volatile uint16_t _edgeCnt[MAX_PINS];
	volatile uint16_t _highCnt[MAX_PINS];
	volatile uint8_t _lastState[MAX_PINS];

	// Computed results (updated in ISR, read in main with interrupts disabled)
	float _freq[MAX_PINS];
	float _duty[MAX_PINS];
};

#endif // IOFUSION_DIGIIN_H
