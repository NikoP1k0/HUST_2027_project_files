#ifndef CAN_FUNCTIONS_H
#define CAN_FUNCTIONS_H

#include <ESP32-TWAI-CAN.hpp>

// Define pins for CAN tranceiver
#define CAN_TX	5
#define CAN_RX	4

// Define variables for CAN-data transmission
extern float battery_volt;
extern float battery_current;
extern float battery_cell_LOW_volt;
extern float battery_cell_HIGH_volt;
extern float battery_cell_AVG_volt;
extern float battery_cell_LOW_temp;
extern float battery_cell_HIGH_temp;
extern float battery_cell_AVG_temp;
extern float battery_cell_ID_HIGH_temp;
extern float battery_cell_ID_LOW_temp;
extern float BMS_temp;

extern float velocity;
extern float distance_travelled;
extern float motor_current;
extern float motor_temp;
extern float motor_controller_temp;
extern float driverRPM;
extern float driver_current;

extern float MPPT1_watt;
extern float MPPT2_watt;
extern float MPPT3_watt;
extern float MPPT_total_watt;

extern CanFrame msg;

void setupCAN();
void sendCAN2steeringWheel(bool brakeLightCondition);
void assignCAN2variable();

#endif
