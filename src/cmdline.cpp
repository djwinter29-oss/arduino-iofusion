#include "cmdline.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace {

template <typename T>
void printError(T message) {
  Serial.print(F("{\"error\":\""));
  Serial.print(message);
  Serial.println(F("\"}"));
}

bool tryParsePositiveFloat(const char* token, float& out) {
  char* endp = nullptr;
  double value = strtod(token, &endp);
  if (endp == token || *endp != '\0' || value <= 0.0) {
    return false;
  }
  out = static_cast<float>(value);
  return true;
}

bool tryParseIntInRange(const char* token, int minValue, int maxValue, int& out) {
  char* endp = nullptr;
  long value = strtol(token, &endp, 10);
  if (endp == token || *endp != '\0') {
    return false;
  }
  if (value < minValue || value > maxValue) {
    return false;
  }
  out = static_cast<int>(value);
  return true;
}

bool tryParseFloat(const char* token, float& out) {
  char* endp = nullptr;
  double value = strtod(token, &endp);
  if (endp == token || *endp != '\0') {
    return false;
  }
  out = static_cast<float>(value);
  return true;
}

void printStatusOk() {
  Serial.println(F("{\"status\":\"ok\"}"));
}

void printHelp() {
  Serial.println(F("{\"help\":\"analog? digital? encoder? pwm-freq <hz> pwm-duty <ch> <pct>\"}"));
}

bool handlePwmFreq(Timer1PWM& pwm, char* const* tokens, uint8_t tokenCount) {
  if (tokenCount < 2) {
    printError(F("missing frequency"));
    return true;
  }
  float freq = 0.0f;
  if (!tryParsePositiveFloat(tokens[1], freq)) {
    printError(F("invalid frequency"));
    return true;
  }
  if (pwm.begin(freq)) {
    printStatusOk();
  } else {
    printError(F("unable to set frequency"));
  }
  return true;
}

bool handlePwmDuty(Timer1PWM& pwm, char* const* tokens, uint8_t tokenCount) {
  if (tokenCount < 3) {
    printError(F("missing duty parameters"));
    return true;
  }
  int channel = 0;
  if (!tryParseIntInRange(tokens[1], 0, 1, channel)) {
    printError(F("invalid channel"));
    return true;
  }
  float duty = 0.0f;
  if (!tryParseFloat(tokens[2], duty)) {
    printError(F("invalid duty"));
    return true;
  }
  pwm.setDuty(static_cast<uint8_t>(channel), duty);
  printStatusOk();
  return true;
}

}  // namespace

CmdLine::CmdLine(AnalogSampler& analog, DigiIn& digi, EncoderGenerator& encoder, Timer1PWM& pwm,
                 const uint8_t* analogPins, uint8_t analogCount, const uint8_t* digitalPins,
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
    (void)handlePwmFreq(_pwm, tokens, tokenCount);
    return;
  }

  if (strcmp(tokens[0], "pwm-duty") == 0) {
    (void)handlePwmDuty(_pwm, tokens, tokenCount);
    return;
  }

  if (strcmp(tokens[0], "help") == 0) {
    printHelp();
    return;
  }

  printError(F("unknown command"));
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
