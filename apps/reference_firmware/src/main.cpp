#include <Arduino.h>

#include "analog_sampler.h"
#include "avr_timer1_pwm.h"
#include "avr_timer2_driver.h"
#include "digital_input_monitor.h"
#include "encoder_generator.h"
#include "firmware_cli.h"
#include "version_info.h"

namespace {
constexpr uint16_t kTimerTickHz = 10000;
constexpr uint16_t kAnalogRequestHz = 500;
static_assert(kTimerTickHz % kAnalogRequestHz == 0,
              "Analog request rate must divide the scheduler tick rate.");
constexpr uint8_t kAnalogTickDivider = kTimerTickHz / kAnalogRequestHz;

Timer2Driver timer2;
AnalogSampler analogSampler;
DigitalInputMonitor digitalInputMonitor;
EncoderGenerator encoder;
Timer1PWM pwm;
uint8_t analogTickDivider = 0;

volatile bool analogOk = false;
volatile bool digitalMonitorOk = false;
volatile bool encoderOk = false;
bool pwmOk = false;
bool timerOk = false;

const uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
const uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};

const AnalogSampler::Config kAnalogConfig = {
    kAnalogPins,
    static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
    5.0f,
};

const DigitalInputMonitor::Config kDigitalMonitorConfig = {
    kDigitalPins,
    static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])),
    500,
  static_cast<float>(kTimerTickHz),
    true,
};

const EncoderGenerator::Config kEncoderConfig = {
    4,
    5,
    6,
    7,
    true,
    false,
};

const Timer1PWM::Config kPwmConfig(100.0f);
const Timer2Driver::Config kTimerConfig(static_cast<float>(kTimerTickHz));
const FirmwareCli::Config kCliConfig = {
    kAnalogPins,
    static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
    kDigitalPins,
    static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])),
};

FirmwareCli firmwareCli(analogSampler, digitalInputMonitor, encoder, pwm, kCliConfig);

void timerTickHandler() {
  if (analogOk) {
    if (++analogTickDivider >= kAnalogTickDivider) {
      analogTickDivider = 0;
      analogSampler.onTick();
    }
  }
  if (digitalMonitorOk) digitalInputMonitor.onTick();
  if (encoderOk) encoder.onTick();
}

void processSerial() {
  firmwareCli.processSerial();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  pinMode(LED_BUILTIN, OUTPUT);

  analogOk = analogSampler.begin(kAnalogConfig);
  if (!analogOk) Serial.println(F("{\"error\":\"analog init failed\"}"));
  analogTickDivider = 0;

  digitalMonitorOk = digitalInputMonitor.begin(kDigitalMonitorConfig);
  if (!digitalMonitorOk) Serial.println(F("{\"error\":\"digital init failed\"}"));

  encoderOk = encoder.begin(kEncoderConfig);
  if (!encoderOk) Serial.println(F("{\"error\":\"encoder init failed\"}"));

  pwmOk = pwm.begin(kPwmConfig);
  if (!pwmOk) {
    Serial.println(F("{\"error\":\"pwm init failed\"}"));
  } else {
    pwm.setDuty(0, 50.0f);
    pwm.setDuty(1, 25.0f);
  }

  timerOk = timer2.begin(kTimerConfig) > 0;
  if (!timerOk) {
    Serial.println(F("{\"error\":\"timer2 init failed\"}"));
  } else {
    timerOk = timer2.attachCallback(timerTickHandler);
    if (!timerOk) {
      timer2.stop();
      Serial.println(F("{\"error\":\"timer2 callback attach failed\"}"));
    }
  }
}

void loop() {
  if (analogOk) analogSampler.sampleIfDue();
  if (digitalMonitorOk) digitalInputMonitor.updateIfReady();
  processSerial();
}