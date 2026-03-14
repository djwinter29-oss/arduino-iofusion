#include <Arduino.h>

#include "analog_sampler.h"
#include "avr_timer1_pwm.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"

AnalogSampler analogSampler;
DigitalInputMonitor digitalInputMonitor;
EncoderGenerator encoder;
Timer1PWM pwm;

const uint8_t kAnalogPins[] = {0, 1};
const uint8_t kDigitalPins[] = {2, 3};

const AnalogSampler::Config kAnalogConfig = {kAnalogPins, 2, 5.0f};
const DigitalInputMonitor::Config kDigitalMonitorConfig = {kDigitalPins, 2, 500, 1000.0f, true};
const EncoderGenerator::Config kEncoderConfig = {8, 11, 12, 13, true, false};
const Timer1PWM::Config kPwmConfig = {1000.0f};

void setup() {
  Serial.begin(115200);

  analogSampler.begin(kAnalogConfig);
  digitalInputMonitor.begin(kDigitalMonitorConfig);

  // pinA=8, pinB=11, up=12, down=13 using pull-up inputs with active-LOW controls
  encoder.begin(kEncoderConfig);

  // Timer1 PWM on Uno pins 9/10
  pwm.begin(kPwmConfig);
  pwm.setDuty(0, 25.0f);
  pwm.setDuty(1, 75.0f);
}

void loop() {
  // In a real app, call onTick() from a hardware timer ISR and keep ISR work minimal.
  analogSampler.onTick();
  digitalInputMonitor.onTick();
  encoder.onTick();

  analogSampler.sampleIfDue();
  digitalInputMonitor.updateIfReady();

  delay(1);
}
