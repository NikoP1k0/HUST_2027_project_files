#ifndef LORA_FUNCTIONS_H
#define LORA_FUNCTIONS_H

#include <LoRa.h>
#include <SPI.h>
#include "CAN_functions.h"

// Define the pins used by the transceiver module
#define SS 10
#define RST 9
#define DIO0 8

// Define pins for miso mosi and clock
#define SCK 13
#define MISO 12
#define MOSI 11

extern SPIClass spiLoRa;

extern float sendInterval;
extern int sf;
extern long sbw;

void setupLoRa();
void sendLoRaData();

#endif
