#ifndef LORA_FUNCTIONS_H
#define LORA_FUNCTIONS_H

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define SS 37
#define RST 9
#define DIO0 48

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