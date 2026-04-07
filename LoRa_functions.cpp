#include "LoRa_functions.h"

float sendInterval = 1.0;     // seconds between sends (used by sender)
int   sf           = 7;       // spreading factor
long  sbw          = 125000;  // bandwidth: 125 kHz

void setupLoRa() {
  Serial.println("Initializing LoRa radio...");

  LoRa.setSPI(SPI);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  while (!LoRa.begin(868E6)) {
    Serial.println("LoRa begin failed, retrying...");
    delay(500);
  }

  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(0xF3);

  Serial.println("LoRa initialized OK");
}

void sendLoRaTestMessage() {
  String msg = "Hello world" + String(millis());

  Serial.print("Packet Sending: ");
  Serial.println(msg);

  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}
