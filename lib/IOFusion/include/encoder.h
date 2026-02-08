// Simple quadrature encoder signal generator
#ifndef IOFUSION_ENCODER_H
#define IOFUSION_ENCODER_H

#include <Arduino.h>

class EncoderGenerator {
public:
	// Initialize generator pins and frequency. Returns true on success.
	bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down);
	// Called from ISR to advance the quadrature state and write outputs.
	void onTick();

	int32_t getPosition();
	bool getDirection();
	// Reset position to initial state (0)
	void reset();

private:
	// Instance state
	uint8_t _pinA = 255;
	uint8_t _pinB = 255;
	uint8_t _state = 0;
	uint8_t _pinUp = 255;
	uint8_t _pinDown = 255;
	volatile int32_t _position = 0;
	volatile bool _directionUp = true;
};

// No global instance here — create an instance in your `main.cpp` as needed.

#endif // IOFUSION_ENCODER_H
