#include "cmdline.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

CmdLine::CmdLine(AnalogSampler& analog,
                 DigiIn& digi,
                 EncoderGenerator& encoder,
                 Timer1PWM& pwm,
                 const uint8_t* analogPins,
                 uint8_t analogCount,
                 const uint8_t* digitalPins,
                 uint8_t digitalCount)
  : _analog(analog),
    _digi(digi),
    _encoder(encoder),
    _pwm(pwm),
    _analogPins(analogPins),
    _analogCount(analogCount),
    _digitalPins(digitalPins),
    _digitalCount(digitalCount) {}

void CmdLine::respondAnalog() {
  Serial.print(F("{"));
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(F("\"a"));
    Serial.print(_analogPins[i]);
    Serial.print(F("\":"));
    Serial.print(_analog.getValue(i), 3);
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
  Serial.println(F("}"));
}

void CmdLine::respondDigital() {
  Serial.print(F("{"));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(F("\"d"));
    Serial.print(_digitalPins[i]);
    Serial.print(F("\":{\"freq\":"));
    Serial.print(_digi.getFrequency(i), 1);
    Serial.print(F(",\"duty\":"));
    Serial.print(_digi.getDutyCycle(i), 1);
    Serial.print(F("}"));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.println(F("}"));
}

void CmdLine::respondEncoder() {
  Serial.print(F("{\"encoder\":{\"direction\":\""));
  Serial.print(_encoder.getDirection() ? F("UP") : F("DOWN"));
  Serial.print(F("\",\"position\":"));
  Serial.print(_encoder.getPosition());
  Serial.println(F("}}"));
}

void CmdLine::handleCommand(char* cmd) {
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
    if (_pwm.begin(freq)) {
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
    _pwm.setDuty(static_cast<uint8_t>(channel), duty);
    Serial.println(F("{\"status\":\"ok\"}"));
    return;
  }

  if (strcmp(tokens[0], "help") == 0) {
    Serial.println(F("{\"help\":\"analog? digital? encoder? pwm-freq <hz> pwm-duty <ch> <pct>\"}"));
    return;
  }

  Serial.println(F("{\"error\":\"unknown command\"}"));
}

void CmdLine::dispatchCommand() {
  if (_cmdLength == 0) return;
  _cmdBuffer[_cmdLength] = '\0';
  handleCommand(_cmdBuffer);
  _cmdLength = 0;
  _cmdBuffer[0] = '\0';
}

void CmdLine::processSerial() {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      dispatchCommand();
      _lastByteTimeMs = 0;
    } else {
      if (_cmdLength < kCmdBufferSize - 1) {
        _cmdBuffer[_cmdLength++] = c;
      }
      _lastByteTimeMs = millis();
    }
  }
  if (_cmdLength > 0 && _lastByteTimeMs != 0) {
    unsigned long now = millis();
    if (now - _lastByteTimeMs >= kCmdIdleTimeoutMs) {
      dispatchCommand();
      _lastByteTimeMs = 0;
    }
  }
}
