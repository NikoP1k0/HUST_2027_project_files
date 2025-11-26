#include "LoRa_functions.h"

// Transmitting frequency
float sendInterval = 1.0;
// Spreading factor (6-12)
int sf = 7;
// Signal bandwidth (20.8e3, 62.5e3, 125e3, and 250e3)
long sbw = 125000;

SPIClass spiLoRa(FSPI);

void setupLoRa() {
  
  Serial.println("LoRa Sender");

  // Begin custom SPI
  spiLoRa.begin(SCK, MISO, MOSI, SS);

  // Setup LoRa transceiver module
  LoRa.setSPI(spiLoRa);
  LoRa.setPins(SS, RST, DIO0);

  //915-938 for Australia
  //863-870 or 433 for Sweden
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }

  // Set spreading factor
  LoRa.setSpreadingFactor(sf);
  // Set bandwidth
  LoRa.setSignalBandwidth(sbw);

  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void sendLoRaData() {

    while(LoRa.beginPacket() == 0) {
      Serial.println("Waiting for radio...");
      delay(100);
    }

    LoRa.beginPacket();
    
    LoRa.print(velocity); LoRa.print(" ");
    LoRa.print(distance_travelled); LoRa.print(" ");
    LoRa.print(battery_volt); LoRa.print(" ");
    LoRa.print(battery_current); LoRa.print(" ");
    LoRa.print(battery_cell_LOW_volt); LoRa.print(" ");
    LoRa.print(battery_cell_HIGH_volt); LoRa.print(" ");
    LoRa.print(battery_cell_AVG_volt); LoRa.print(" ");
    LoRa.print(battery_cell_LOW_temp); LoRa.print(" ");
    LoRa.print(battery_cell_HIGH_temp); LoRa.print(" ");
    LoRa.print(battery_cell_AVG_temp); LoRa.print(" ");
    LoRa.print(battery_cell_ID_HIGH_temp); LoRa.print(" ");
    LoRa.print(battery_cell_ID_LOW_temp); LoRa.print(" ");
    LoRa.print(BMS_temp); LoRa.print(" ");
    LoRa.print(motor_current); LoRa.print(" ");
    LoRa.print(motor_temp); LoRa.print(" ");
    LoRa.print(motor_controller_temp); LoRa.print(" ");
    LoRa.print(MPPT1_watt); LoRa.print(" ");
    LoRa.print(MPPT2_watt); LoRa.print(" ");
    LoRa.print(MPPT3_watt); LoRa.print(" ");
    LoRa.print(MPPT_total_watt);

    LoRa.endPacket();
    Serial.println("CAN sent with LoRa.");
}
