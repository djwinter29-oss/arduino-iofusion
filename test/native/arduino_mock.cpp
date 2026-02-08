#include "Arduino.h"

unsigned long mockMillis = 0;
uint8_t mockPortIn[8] = {0};
uint8_t mockPortOut[8] = {0};
int mockAnalogValues[16] = {0};
MockSerial Serial;
