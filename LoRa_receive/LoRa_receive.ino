/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include "LoRa_functions.h"

// Kalman filter variables
float kalmanGain = 0.1;
float estimateError = 3.0;
float measurementError = 3.0;
float estimatedRSSI = -80.0;

float packetRSSI;
String LoRaData;

void setup() {
  //initialize Serial Monitor
  delay(500);
  Serial.begin(57600);
  while (!Serial);
  
  setupLoRa();
}

float kalmanFilter(float measurement) {

  // Prediction step
  float prediction = estimatedRSSI;
  estimateError += measurementError;

  // Update step
  kalmanGain = estimateError / (estimateError + measurementError);
  estimatedRSSI = prediction + kalmanGain * (measurement - prediction);
  estimateError = (1 - kalmanGain) * estimateError;

  return estimatedRSSI;
}

void loop() {

  // Try to parse packet
  int packetSize = LoRa.parsePacket();

  if (packetSize) {    // received a packet
    Serial.println("Message received!");
    while (LoRa.available()) {    // read packet
      LoRaData = LoRa.readString();
      Serial.println("LoRa data: " + LoRaData);
    }

    // Extract velocity from the LoRa data
    /* int spaceIndex = LoRaData.indexOf(' ');
    if (spaceIndex != -1) {
      String velocityStr = LoRaData.substring(0, spaceIndex);
      float velocity = velocityStr.toFloat();

      // Calculate distance (assuming velocity is in m/s and you want distance in meters)
      float distance = velocity * 3; // Modify this calculation based on the estimation

      // Adjust parameters based on distance
      adjustParameters(distance);
    } */

    // RSSI of packet
    packetRSSI = LoRa.packetRssi();
    Serial.printf("RSSI: %.4f\n", packetRSSI);

    // Apply Kalman Filter
    estimatedRSSI = kalmanFilter(packetRSSI);
    Serial.printf("Kalman RSSI: %.4f\n", estimatedRSSI);
  }
}
