/*
 * HUST LoRa - STM32 Nucleo-L4R6G + RFM95W
 * Full duplex LoRa communication - can send and receive
 * Add CAN/SD/LED when ready.
 */
#include <Arduino.h>
#include "LoRa_functions.h"

void setup() {
  delay(2000);

  Serial.begin(115200);
  while (!Serial)
    ;  // Wait for Serial (remove if using standalone)

  setupLoRa();
  Serial.println("LoRa Transceiver Ready - Sending and Receiving...");
}

void loop() {
  // Check for incoming messages (non-blocking)
  receiveLoRaMessage();
  
  // Send a message periodically
  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastSendTime >= (unsigned long)(sendInterval * 1000)) {
    sendLoRaTestMessage();
    lastSendTime = currentTime;
  }
}
