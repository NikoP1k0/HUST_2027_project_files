#include "LoRa_functions.h"

// Transmitting frequency (seconds)
float sendInterval = 1.0;
// Spreading factor (6-12)
int sf = 7;
// Signal bandwidth (20.8e3, 62.5e3, 125e3, and 250e3)
long sbw = 125000;

void setupLoRa() {
  Serial.println("LoRa Sender (STM32)");

  // Use default SPI (SPI1 on Nucleo - D11=MOSI, D12=MISO, D13=SCK)
  LoRa.setSPI(SPI);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // 915 MHz (Australia). Use 868E6 for EU, 433E6 for 433 MHz band
  while (!LoRa.begin(868E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(0xF3);  // Match with receiver
  Serial.println("LoRa Initializing OK!");
}

void sendLoRaTestMessage() {
  static uint32_t counter = 0;

  if (!LoRa.beginPacket()) {
    Serial.println("Waiting for radio...");
    return;
  }

  LoRa.print("Hello LoRa #");
  LoRa.print(counter++);
  LoRa.endPacket();
  Serial.println("Packet sent.");
}

void receiveLoRaMessage() {
  // Check if a packet has been received
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet (");
    Serial.print(packetSize);
    Serial.print(" bytes): ");
    
    // Read packet data
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    
    // Print RSSI and SNR
    Serial.print(" | RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm | SNR: ");
    Serial.print(LoRa.packetSnr());
    Serial.println(" dB");
  }
}