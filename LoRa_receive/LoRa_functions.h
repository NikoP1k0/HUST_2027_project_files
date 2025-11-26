#ifndef LORA_FUNCTIONS_H
#define LORA_FUNCTIONS_H

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define SS 5
#define RST 14
#define DIO0 2

// Variables
extern long sbw; 
extern int sf;
extern int updateFlag;
extern long newSbw;
extern int newSf;

void setupLoRa();
void adjustParameters(float distance);
void sendParameters();

#endif