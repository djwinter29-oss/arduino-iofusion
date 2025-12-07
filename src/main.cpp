#include <Arduino.h>

#include "timer.h"
#include "version_info.h"
#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

namespace {
  constexpr float kTimerTickHz = 10000.0f; // Timer2 tick frequency

  Timer2Driver timer2;
  AnalogSampler analogSampler;
  DigiIn digiIn;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  const uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  const uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};
  // pwm 9,10

  constexpr size_t kCmdBufferSize = 64;
  constexpr unsigned long kCmdIdleTimeoutMs = 75;
  char cmdBuffer[kCmdBufferSize];
  size_t cmdLength = 0;
  unsigned long lastByteTimeMs = 0;

  void timerTickHandler() {
    analogSampler.onTick();
    digiIn.onTick();
    encoder.onTick();
  }

  void respondAnalog() {
    Serial.print(F("{"));
    const uint8_t count = analogSampler.getChannelCount();
    for (uint8_t i = 0; i < count; ++i) {
      Serial.print(F("\"a"));
      Serial.print(kAnalogPins[i]);
      Serial.print(F("\":"));
      Serial.print(analogSampler.getValue(i), 3);
      if (i + 1 < count) Serial.print(F(","));
    }
    Serial.println(F("}"));
  }

  void respondDigital() {
    Serial.print(F("{"));
    const uint8_t count = digiIn.getPinCount();
    for (uint8_t i = 0; i < count; ++i) {
      Serial.print(F("\"d"));
      Serial.print(kDigitalPins[i]);
      Serial.print(F("\":{\"freq\":"));
      Serial.print(digiIn.getFrequency(i), 1);
      Serial.print(F(",\"duty\":"));
      Serial.print(digiIn.getDutyCycle(i), 1);
      Serial.print(F("}"));
      if (i + 1 < count) Serial.print(F(","));
    }
    Serial.println(F("}"));
  }

  void respondEncoder() {
    Serial.print(F("{\"encoder\":{\"direction\":\""));
    Serial.print(encoder.getDirection() ? F("UP") : F("DOWN"));
    Serial.print(F("\",\"position\":"));
    Serial.print(encoder.getPosition());
    Serial.println(F("}}"));
  }

  void handleCommand(char* cmd) {
    while (*cmd && isspace(static_cast<unsigned char>(*cmd))) ++cmd;
    if (*cmd == '\0') return;

    char* tokens[4];
    uint8_t tokenCount = 0;
    char* tok = strtok(cmd, " ");
    while (tok && tokenCount < 4) {
      tokens[tokenCount++] = tok;
      tok = strtok(nullptr, " ");
    }
    if (tokenCount == 0) return;

    for (char* p = tokens[0]; *p; ++p) {
      *p = tolower(static_cast<unsigned char>(*p));
    }

    if (strcmp(tokens[0], "analog?") == 0) {
      respondAnalog();
      return;
    }

    if (strcmp(tokens[0], "digital?") == 0) {
      respondDigital();
      return;
    }

    if (strcmp(tokens[0], "encoder?") == 0) {
      respondEncoder();
      return;
    }

    if (strcmp(tokens[0], "pwm-freq") == 0) {
      if (tokenCount < 2) {
        Serial.println(F("{\"error\":\"missing frequency\"}"));
        return;
      }
      float freq = atof(tokens[1]);
      if (freq <= 0.0f) {
        Serial.println(F("{\"error\":\"invalid frequency\"}"));
        return;
      }
      if (pwm.begin(freq)) {
        Serial.println(F("{\"status\":\"ok\"}"));
      } else {
        Serial.println(F("{\"error\":\"unable to set frequency\"}"));
      }
      return;
    }

    if (strcmp(tokens[0], "pwm-duty") == 0) {
      if (tokenCount < 3) {
        Serial.println(F("{\"error\":\"missing duty parameters\"}"));
        return;
      }
      int channel = atoi(tokens[1]);
      if (channel < 0 || channel > 1) {
        Serial.println(F("{\"error\":\"invalid channel\"}"));
        return;
      }
      float duty = atof(tokens[2]);
      pwm.setDuty(static_cast<uint8_t>(channel), duty);
      Serial.println(F("{\"status\":\"ok\"}"));
      return;
    }

    if (strcmp(tokens[0], "help") == 0) {
      Serial.println(F("{\"help\":\"analog? digital? encoder? pwm-freq <hz> pwm-duty <ch> <pct>\"}"));
      return;
    }

    Serial.println(F("{\"error\":\"unknown command\"}"));
  }

  void dispatchCommand() {
    if (cmdLength == 0) return;
    cmdBuffer[cmdLength] = '\0';
    handleCommand(cmdBuffer);
    cmdLength = 0;
    cmdBuffer[0] = '\0';
  }

  void processSerial() {
    while (Serial.available() > 0) {
      char c = static_cast<char>(Serial.read());
      if (c == '\r' || c == '\n') {
        dispatchCommand();
        lastByteTimeMs = 0;
      } else {
        if (cmdLength < kCmdBufferSize - 1) {
          cmdBuffer[cmdLength++] = c;
        }
        lastByteTimeMs = millis();
      }
    }
    if (cmdLength > 0 && lastByteTimeMs != 0) {
      unsigned long now = millis();
      if (now - lastByteTimeMs >= kCmdIdleTimeoutMs) {
        dispatchCommand();
        lastByteTimeMs = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  pinMode(LED_BUILTIN, OUTPUT);

  analogSampler.begin(kAnalogPins, static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])));

  digiIn.begin(kDigitalPins, static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])), 500, kTimerTickHz, true);

  encoder.begin(4, 5, 6, 7);

  pwm.begin(100.0f);
  pwm.setDuty(0, 50.0f);
  pwm.setDuty(1, 25.0f);

  timer2.beginHz(kTimerTickHz);
  timer2.attachCallback(timerTickHandler);
}

void loop() {
  analogSampler.sampleIfDue();
  processSerial();
}
