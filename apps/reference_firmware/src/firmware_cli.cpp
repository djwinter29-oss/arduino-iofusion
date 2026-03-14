#include "firmware_cli.h"

#include <ctype.h>
#include <string.h>

namespace {

template <typename T>
void printError(T message) {
  Serial.print(F("{\"error\":\""));
  Serial.print(message);
  Serial.println(F("\"}"));
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

bool tryParseFixed3(const char* token, bool allowSign, int32_t& out) {
  bool negative = false;
  if (*token == '+' || *token == '-') {
    if (!allowSign) return false;
    negative = (*token == '-');
    ++token;
  }
  if (*token == '\0') return false;

  uint32_t integerPart = 0;
  uint32_t fractionalPart = 0;
  uint8_t fractionalDigits = 0;
  bool seenDigit = false;
  bool seenDot = false;

  for (const char* p = token; *p != '\0'; ++p) {
    char c = *p;
    if (c >= '0' && c <= '9') {
      uint8_t digit = static_cast<uint8_t>(c - '0');
      seenDigit = true;
      if (seenDot) {
        if (fractionalDigits >= 3) return false;
        fractionalPart = (fractionalPart * 10U) + digit;
        ++fractionalDigits;
      } else {
        if (integerPart > 999999U) return false;
        integerPart = (integerPart * 10U) + digit;
      }
      continue;
    }
    if (c == '.' && !seenDot) {
      seenDot = true;
      continue;
    }
    return false;
  }

  if (!seenDigit) return false;
  while (fractionalDigits < 3) {
    fractionalPart *= 10U;
    ++fractionalDigits;
  }

  uint32_t scaled = (integerPart * 1000U) + fractionalPart;
  if (scaled > 2147483647UL) return false;
  out = negative ? -static_cast<int32_t>(scaled) : static_cast<int32_t>(scaled);
  return true;
}

bool tryParsePositiveFixed3(const char* token, uint32_t& out) {
  int32_t scaled = 0;
  if (!tryParseFixed3(token, false, scaled) || scaled <= 0) return false;
  out = static_cast<uint32_t>(scaled);
  return true;
}

bool tryParseSignedFixed3(const char* token, int32_t& out) {
  return tryParseFixed3(token, true, out);
}

void printThreeDigits(uint16_t value) {
  char buffer[4];
  buffer[0] = static_cast<char>('0' + ((value / 100U) % 10U));
  buffer[1] = static_cast<char>('0' + ((value / 10U) % 10U));
  buffer[2] = static_cast<char>('0' + (value % 10U));
  buffer[3] = '\0';
  Serial.print(buffer);
}

void printMillivoltsAsVolts(uint16_t millivolts) {
  Serial.print(millivolts / 1000U);
  Serial.print('.');
  printThreeDigits(static_cast<uint16_t>(millivolts % 1000U));
}

void printDeciScaled(uint32_t deciValue) {
  Serial.print(deciValue / 10U);
  Serial.print('.');
  Serial.print(deciValue % 10U);
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
  uint32_t freqMilliHz = 0;
  if (!tryParsePositiveFixed3(tokens[1], freqMilliHz)) {
    printError(F("invalid frequency"));
    return true;
  }
  float freq = static_cast<float>(freqMilliHz) / 1000.0f;
  if (pwm.begin(Timer1PWM::Config{freq})) {
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
  int32_t dutyMilliPercent = 0;
  if (!tryParseSignedFixed3(tokens[2], dutyMilliPercent)) {
    printError(F("invalid duty"));
    return true;
  }
  float duty = static_cast<float>(dutyMilliPercent) / 1000.0f;
  pwm.setDuty(static_cast<uint8_t>(channel), duty);
  printStatusOk();
  return true;
}

}  // namespace

FirmwareCli::FirmwareCli(AnalogSampler& analog, DigitalInputMonitor& digitalMonitor,
                         EncoderGenerator& encoder, Timer1PWM& pwm, const Config& config)
    : FirmwareCli(analog, digitalMonitor, encoder, pwm, config.analogPins, config.analogCount,
                  config.digitalPins, config.digitalCount) {}

FirmwareCli::FirmwareCli(AnalogSampler& analog, DigitalInputMonitor& digitalMonitor,
                         EncoderGenerator& encoder, Timer1PWM& pwm,
                         const uint8_t* analogPins, uint8_t analogCount,
                         const uint8_t* digitalPins, uint8_t digitalCount)
    : _analog(analog),
      _digitalMonitor(digitalMonitor),
      _encoder(encoder),
      _pwm(pwm),
      _analogPins(analogPins),
      _analogCount(analogCount),
      _digitalPins(digitalPins),
      _digitalCount(digitalCount) {}

void FirmwareCli::respondAnalog() {
  Serial.print(F("{"));
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(F("\"a"));
    Serial.print(_analogPins[i]);
    Serial.print(F("\":"));
    printMillivoltsAsVolts(_analog.getMillivolts(i));
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
  Serial.println(F("}"));
}

void FirmwareCli::respondDigital() {
  Serial.print(F("{\"frameSeq\":"));
  Serial.print(_digitalMonitor.getFrameSequence());
  Serial.print(F(",\"stale\":"));
  Serial.print(_digitalMonitor.isFrameStale() ? F("true") : F("false"));
  Serial.print(F(",\"overrunTicks\":"));
  Serial.print(_digitalMonitor.getOverrunCount());
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(F(",\"d"));
    Serial.print(_digitalPins[i]);
    Serial.print(F("\":{\"freq\":"));
    printDeciScaled((_digitalMonitor.getFrequencyMilliHz(i) + 50U) / 100U);
    Serial.print(F(",\"duty\":"));
    printDeciScaled(_digitalMonitor.getDutyPermille(i));
    Serial.print(F("}"));
  }
  Serial.println(F("}"));
}

void FirmwareCli::respondEncoder() {
  Serial.print(F("{\"encoder\":{\"direction\":\""));
  Serial.print(_encoder.getDirection() ? F("UP") : F("DOWN"));
  Serial.print(F("\",\"position\":"));
  Serial.print(_encoder.getPosition());
  Serial.println(F("}}"));
}

void FirmwareCli::handleCommand(char* cmd) {
  while (*cmd && isspace(static_cast<unsigned char>(*cmd))) ++cmd;
  if (*cmd == '\0') return;

  char* tokens[4];
  uint8_t tokenCount = 0;
  char* tok = strtok(cmd, " ");
  while (tok && tokenCount < 4) {
    tokens[tokenCount++] = tok;
    tok = strtok(nullptr, " ");
  }

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

void FirmwareCli::dispatchCommand() {
  if (_cmdLength == 0) return;
  _cmdBuffer[_cmdLength] = '\0';
  handleCommand(_cmdBuffer);
  _cmdLength = 0;
  _cmdBuffer[0] = '\0';
}

void FirmwareCli::processSerial() {
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