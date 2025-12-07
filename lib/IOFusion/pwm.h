// Timer1-based PWM driver for two hardware channels (OC1A / OC1B)
#ifndef IOFUSION_PWM_H
#define IOFUSION_PWM_H

#include <Arduino.h>

class Timer1PWM {
public:
	Timer1PWM();
	// Configure Timer1 to generate PWM on OC1A/OC1B at the requested frequency.
	// Duty for each channel must be set separately via setDuty().
	bool begin(float freqHz);
	// Update duty cycle (0..100%) for a specific channel. 0% keeps the pin low.
	void setDuty(uint8_t channel, float percent);
	// Stop PWM and release pins
	void stop();

private:
	uint16_t _top = 0; // ICR1 top value
	uint16_t _presBits = 0; // CS bits in TCCR1B
	float _dutyPercent[2] = {0.0f, 0.0f};
	bool _configured = false;
	uint16_t percentToCounts(float percent, uint16_t top) const;
	void _applyDuty(uint8_t channel, uint16_t value);
};

#endif // IOFUSION_PWM_H
