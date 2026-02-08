// Simple analog sampler for AVR/Arduino
#ifndef IOFUSION_ANALOG_H
#define IOFUSION_ANALOG_H

#include <Arduino.h>

class AnalogSampler {
public:
	AnalogSampler();
	// begin with an array of analog channel indices (each 0..5) and number of channels (1..6)
	// `channels` points to a buffer of `count` uint8_t entries that will be copied.
	bool begin(const uint8_t* channels, uint8_t count);
	// Called from ISR to request a sampling round
	void onTick();
	// Called from loop() to perform pending sampling and update averages
	void sampleIfDue();

	// Accessors
	uint8_t getChannelCount() const;
	// Returns measured voltage in volts (e.g. 0.0 .. Vref)
	float getValue(uint8_t idx) const;
	// Configure reference voltage used for scaling (default 5.0V)
	void setVref(float vref);

private:
	static const uint8_t MAX_CHANNELS = 6;
	uint8_t _channels[MAX_CHANNELS];
	uint8_t _channelCount = 0;
	volatile bool _sampleRequested = false;
	int _lastValues[MAX_CHANNELS];
	float _vref = 5.0f;
};

#endif // IOFUSION_ANALOG_H
