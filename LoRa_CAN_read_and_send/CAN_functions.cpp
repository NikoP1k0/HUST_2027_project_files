#include "CAN_functions.h"
#include "LED_functions.h"

// Define variables for CAN-data transmission
float battery_volt;
float battery_current;
float battery_cell_LOW_volt;
float battery_cell_HIGH_volt;
float battery_cell_AVG_volt;
float battery_cell_LOW_temp;
float battery_cell_HIGH_temp;
float battery_cell_AVG_temp;
float battery_cell_ID_HIGH_temp;
float battery_cell_ID_LOW_temp;
float BMS_temp;

float velocity;
float distance_travelled;
float motor_current;
float motor_temp;
float motor_controller_temp;
float driverRPM;
float driver_current;

float MPPT1_watt;
float MPPT2_watt;
float MPPT3_watt;
float MPPT_total_watt;

CanFrame msg;

void setupCAN() {

  // Set pins
	ESP32Can.setPins(CAN_TX, CAN_RX);
	
  // You can set custom size for the queues - these are default
  ESP32Can.setRxQueueSize(5);
	ESP32Can.setTxQueueSize(5);

  // .setSpeed() and .begin() functions require to use TwaiSpeed enum,
  // but you can easily convert it from numerical value using .convertSpeed()
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

  if(ESP32Can.begin()) {
    Serial.println("CAN bus started!");
  } else {
    Serial.println("CAN bus failed!");
  }

  if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 5, 5)) {
    Serial.println("CAN bus speed 500!");
  } else {
    Serial.println("CAN bus failed!");
  }
}

void sendCAN2steeringWheel(bool brakeLightCondition) {

    // Send a 0 if brake light is off, and a 1 if it is on
    int bit;
    (brakeLightCondition) ? bit = 1 : bit = 0;

    CanFrame obdFrame         = {0};
    obdFrame.identifier       = 0x175; // Default OBD2 address;
    obdFrame.extd             = 0;
    obdFrame.data_length_code = 8;
    obdFrame.data[0]          = bit;
    obdFrame.data[1]          = 0xAA;
    obdFrame.data[2]          = 0xAA;
    obdFrame.data[3]          = 0xAA; // Best to use 0xAA (0b10101010) instead of 0
    obdFrame.data[4]          = 0xAA; // CAN works better this way as it needs
    obdFrame.data[5]          = 0xAA; // to avoid bit-stuffing
    obdFrame.data[6]          = 0xAA;
    obdFrame.data[7]          = 0xAA;
    // Accepts both pointers and references
    ESP32Can.writeFrame(obdFrame); // timeout defaults to 1 ms
}

void assignCAN2variable() {

  // For MPPT power out
  double voltOut = ((double)(msg.data[4] * 256 + msg.data[5]))/100;
  double currentOut;
    if (msg.data[6] >= 128)
      currentOut = ((double)((msg.data[6] - 256) * 256 + msg.data[7]))*0.0005;
    else
      currentOut = ((double)(msg.data[6] * 256 + msg.data[7]))*0.0005;
  
  switch(msg.identifier) {
    case 0x402:
      motor_current = *((float*)(msg.data+4));
      break;
    case 0x403:
      velocity = *((float*)(msg.data+4));
      break;
    case 0x40B:
      motor_controller_temp = *((float*)(msg.data+4));
      motor_temp = *((float*)(msg.data));
      break;
    case 0x40E:
      distance_travelled = *((float*)(msg.data));
      break;

    // Battery temperatures
    case 0x601:
      BMS_temp = msg.data[1];
      battery_cell_HIGH_temp = msg.data[2];
      battery_cell_LOW_temp = msg.data[3];
      battery_cell_AVG_temp = msg.data[4];
      battery_cell_ID_HIGH_temp = msg.data[5];
      battery_cell_ID_LOW_temp = msg.data[6];
      break;

    // Battery voltage and current
    case 0x602:
      if(msg.data[0] >= 128)
        battery_current = ((double)(msg.data[0] - 256) * 256 + msg.data[1])/10;
      else
        battery_current = ((double)(msg.data[0] * 256 + msg.data[1]))/10;
      battery_volt = ((double)(msg.data[2] * 256 + msg.data[3]))/10;
      battery_cell_LOW_volt = ((double)(msg.data[4] * msg.data[5]))/10000;
      battery_cell_HIGH_volt = ((double)(msg.data[6] * msg.data[7]))/10000;
      break;

    case 0x603:
      battery_cell_AVG_volt = ((double)(msg.data[0] * 256 + msg.data[1]))/10000;
      break;
      
    //MPPT power out (watt)
    case 0x200:
      MPPT1_watt = voltOut * currentOut;
      break;
    case 0x210:
      MPPT2_watt = voltOut * currentOut;
      break;
    case 0x220:
      MPPT3_watt = voltOut * currentOut;
      break;
    
    //Blinkers, hazards, and brake
    case 0x176:
      left_blinker = msg.data[0];
      right_blinker = msg.data[1];
      hazard_light = msg.data[2];
      brake_light = msg.data[3];
      //Serial.printf("left: %d, right %d, hazard: %d, brake: %d\n", left_blinker, right_blinker, hazard_light, brake_light);

    case 0x501:
      driver_current = *((float*)(msg.data+4));
      driverRPM = *((float*)(msg.data));
      //Serial.printf("Driver current: %.2f, Driver RPM: %.2f", driver_current, driverRPM);
      //Serial.println();
      
    default:
      //Serial.println("No matching identifier");
      break;
  }
  MPPT_total_watt = MPPT1_watt + MPPT2_watt + MPPT3_watt;
}
