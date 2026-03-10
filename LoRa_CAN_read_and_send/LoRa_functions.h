#ifndef LORA_FUNCTIONS_H
#define LORA_FUNCTIONS_H

#include <LoRa.h>
#include <SPI.h>

// Pins for Nucleo-L4R6G + RFM95W
// NSS=D10, SCK=D13, MISO=D12, MOSI=D11, RST=D9, DIO0=D2
#define LORA_SS   10   // NSS (Chip Select)
#define LORA_RST  9    // Reset
#define LORA_DIO0 2    // DIO0 (interrupt)
#define LORA_SCK  13
#define LORA_MISO 12
#define LORA_MOSI 11

extern float sendInterval;
extern int sf;
extern long sbw;

void setupLoRa();
void sendLoRaTestMessage();  // Sends simple test message (use until CAN is on STM32)
void receiveLoRaMessage();  // Checks for and prints received LoRa messages

#endif