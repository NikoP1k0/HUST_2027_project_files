#include <Arduino.h>
#include "LoRa_functions.h"

void setup() {
  delay(2000);  // small startup delay

  Serial.begin(115200);
  while (!Serial) {
    // wait for Serial
  }

  Serial.println("LoRa Sender (STM32)");
  setupLoRa();
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastSendTime >= (unsigned long)(sendInterval * 1000)) {
    sendLoRaTestMessage();
    lastSendTime = currentTime;
  }

  // small delay to avoid tight loop
  delay(10);
}