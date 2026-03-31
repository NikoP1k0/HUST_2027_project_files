#include <Arduino.h>
#include "LoRa_functions.h"

void setup() {
  delay(2000);  // small startup delay

  Serial.begin(115200);
  while (!Serial) {
    // wait for Serial
  }

  Serial.println("LoRa Receiver (STM32)");
  setupLoRa();
}

void loop() {
  receiveLoRaMessage();
  // small delay to avoid tight loop
  delay(10);
}