#include <Arduino.h>

#include "timer.h"
#include "version_info.h"

Timer2Driver t2;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  // Start Timer2 at 40KHz for ADC sampling
  t2.beginHz(40000);

  
}

unsigned long lastPrint = 0;

void loop() {

}
