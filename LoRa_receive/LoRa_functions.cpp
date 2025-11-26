#include "LoRa_functions.h"

long sbw = 125000; // 20800, 62500, 125000, 250000
int sf = 7;       // 6, 7, 8, 9, 10, 11, 12
int updateFlag = 1;
long newSbw;
int newSf;

void setupLoRa() {

  Serial.println("LoRa Receiver");

  // Setup LoRa transceiver module pins
  LoRa.setPins(SS, RST, DIO0);

  //915-938 for Australia
  //863-870 or 433 for Sweden
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSpreadingFactor(sf); 
  Serial.println("LoRa Initializing OK!");
}

void sendParameters() {
  LoRa.beginPacket();
  LoRa.print("PARAM ");
  LoRa.print(sf);
  LoRa.print(" ");
  LoRa.print(sbw);
  LoRa.endPacket();
  Serial.println("Sent parameters to sender");
}

void adjustParameters(float distance) {
  if (distance >= 0 && distance < 43.75) {
    sf = 7;
    sbw = 125000;
  } else if (distance >= 43.75 && distance < 56.25) {
    sf = 7;
    sbw = 62500;
  } else if (distance >= 56.25 && distance < 68.75) {
    sf = 8;
    sbw = 125000;
  } else if (distance >= 68.75 && distance < 81.25) {
    sf = 8;
    sbw = 62500;
  } else if (distance >= 81.25 && distance < 93.75) {
    sf = 8;
    sbw = 125000;
  } else if (distance >= 93.75 && distance < 106.25) {
    sf = 9;
    sbw = 62500;
  } else if (distance >= 106.25 && distance < 118.75) {
    sf = 7;
    sbw = 62500;
  } else if (distance >= 118.75) {
    sf = 8;
    sbw = 125000;
  }

  // Only update and send parameters if there is a change or if its the first time
  if (newSf != sf || newSbw != sbw || updateFlag == 1) {
    updateFlag = 0;
    sf = newSf;
    sbw = newSbw;
    LoRa.setSignalBandwidth(sbw);
    LoRa.setSpreadingFactor(sf);
    sendParameters();
  }
}