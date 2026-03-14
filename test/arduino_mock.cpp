#include "Arduino.h"

unsigned long mockMillis = 0;
uint8_t mockPortIn[8] = {0};
uint8_t mockPortOut[8] = {0};
uint8_t mockPinModes[64] = {0};
int mockAnalogValues[16] = {0};
int mockNullInputPort = -1;
int mockNullOutputPort = -1;
int mockZeroMaskPin = -1;
MockSerial Serial;