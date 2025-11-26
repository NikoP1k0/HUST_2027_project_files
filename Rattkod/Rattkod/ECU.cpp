#include <Arduino.h>
#include <SPI.h>
#include "Platform.h"
#include "App_Common.h"
#include "ECU.h"
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <ESP32-TWAI-CAN.hpp>


CanFrame msg;

extern Adafruit_MCP23X17 mcp;
extern bool InStartScreen, CCactive;

extern Gpu_Hal_Context_t host, *phost;

extern float MotorCurrent, MotorVoltage, VehicleVelocity, MotorRPM, DriverCurrent, DriverVelocity, Amps, Distance, HeatSinkTemp, MotorTemp;

extern uint8_t InternalTempBMS, CellHighTemperature, CellLowTemperature, CellAverageTemperature, CellHighTemperature_ID, CellLowTemperature_ID;
extern double PackCurrent, PackVoltage, LowCellVoltage, HighCellVoltage, AverageCellVoltage;

extern double MPPTInputVoltage1, MPPTInputCurrent1, MPPTOutputVoltage1, MPPTOutputCurrent1, MPPTOutputPower1;
extern double MPPTInputVoltage2, MPPTInputCurrent2, MPPTOutputVoltage2, MPPTOutputCurrent2, MPPTOutputPower2;
extern double MPPTInputVoltage3, MPPTInputCurrent3, MPPTOutputVoltage3, MPPTOutputCurrent3, MPPTOutputPower3;
extern double MPPTInputVoltage4, MPPTInputCurrent4, MPPTOutputVoltage4, MPPTOutputCurrent4, MPPTOutputPower4;
extern double TotMPPT;
extern int CCSpeed;
extern int CCPower;
extern int LightState;

extern float CurrentConsum, AverageConsum;

extern float Gas, MaxGas, MinGas, Brake, MaxBrake, MinBrake, TargetRPM, TargetCurrent;
extern int GasLimiter;
extern int GearSelect;
extern int brakepedal;

extern unsigned long prevmsg, prevCons,prevButton;

extern bool CCUp, CCDown, LeftTurnButton, RightTurnButton, HazardButton, GearButton; //Ändrade till bool här med
extern bool Left_Turn, Right_Turn, Hazard, SolarActive, SolarPrecharge;
extern bool DriveButton;
extern bool Drive;
extern bool BlinkState;
extern bool SafeState;
extern bool MCerror;
extern bool BMSerror;
extern bool HighTemp;

void CalibratePot(){
  Gas = analogRead(GAS_POTENTIOMETER);
  Brake = analogRead(BRAKE_POTENTIOMETER); 
  DriveButton = digitalRead(13);

  if (Gas < MinGas&&Gas>MinGas-100) {
      MinGas = Gas;
  } else if (Gas > MaxGas&&Gas<MaxGas+100) {
      MaxGas = Gas;
  }
  if (Brake < MinBrake&&Brake>MinBrake-100) { 
    MinBrake = Brake;
  } else if (Brake > MaxBrake&&Brake<MaxBrake+100) {
    MaxBrake = Brake;
  }
  
}
void ReadPot() {  //Läser Gas och Broms och omvandlar till float 
  Gas = analogRead(GAS_POTENTIOMETER);
  Brake = analogRead(BRAKE_POTENTIOMETER); 
  DriveButton = digitalRead(DRIVE_SELECT_BUTTON);
  if(Gas<500||Gas>3500||Brake<500||Brake>3500){
    Gas=0;
    Brake=0;
  }else{
    /*if(Gas<MinGas+150){
      Gas=MinGas;
    }
    if(Brake<MinBrake+100){
      Brake=MinBrake;
    }
    if(Gas>MaxGas){
      Gas=MaxGas;
    }
    if(Brake>MaxBrake){
      Brake=MaxBrake;
    }*/
    if(Gas<MinGas+150){
      Gas=MinGas+150;
    }else if(Gas>MaxGas){
      Gas=MaxGas;
    }
    Gas = mapFloat(Gas, (float)(MinGas+150), (float)MaxGas, 0, GasLimiter / 5.0);

    if(Brake<MinBrake+150){
      Brake=MinBrake+150;
    }else if(Brake>MaxBrake){
      Brake=MaxBrake;
    }
    Brake = mapFloat(Brake, (float)(MinBrake+150), (float)MaxBrake ,0, 1);//(float)MaxBrake, 0, 0.8); //Ändra till 1 med nya batteriet
  }
  if(DriveButton){
    if (Brake != 0) {
      CCactive = false;
      TargetCurrent = Brake;
      TargetRPM = -100;
    } else if (Gas != 0 && CCactive==0) {//Ta bort CCactive för farthållare
      //CCactive = false; //Avkommentera för farthållare, kommentera för ström
      TargetCurrent = Gas;
      TargetRPM = 20000;
    } else if (!CCactive) {
      TargetCurrent = 0;
      TargetRPM = 0;
    }
  }else{
    CCactive = false;
    TargetCurrent = 0;
    TargetRPM = 0;
  }
}



void ReadButtons() {
  LeftTurnButton = digitalRead(LEFT_TURN_BUTTON);
  RightTurnButton = digitalRead(RIGHT_TURN_BUTTON); 
  HazardButton = digitalRead(HAZARD_BUTTON);
  CCUp = digitalRead(CC_UP_BUTTON);
  CCDown = digitalRead(CC_DOWN_BUTTON);
  GearButton = digitalRead(OK_GEAR_BUTTON);

  if(LeftTurnButton||RightTurnButton||HazardButton||CCUp||CCDown||GearButton){
    prevButton=millis()+500;
  }

  if (GearButton) { 
    GasLimiter++;
    if (GasLimiter == 6) {
      GasLimiter = 1;  
    }
  }

  if (RightTurnButton) {
    Right_Turn = !Right_Turn;
    Left_Turn = false;
  } else if (HazardButton) {
    Hazard = !Hazard;
    Left_Turn = false;
    Right_Turn = false;
  } else if (LeftTurnButton) {
    Left_Turn = !Left_Turn;
    Right_Turn = false;
  }

  //CC
  /* Farthållare:
  if (CCactive) { 
    if (CCDown && CCSpeed > 0) {
      CCSpeed = CCSpeed - 1;
      while (CCSpeed % 5 != 0) {
        CCSpeed--;
      }
      TargetRPM = CCSpeed * 9.51;
    } else if (CCUp) {
      CCSpeed = CCSpeed + 1;
      while (CCSpeed % 5 != 0) {
        CCSpeed++;
      }
      TargetRPM = CCSpeed * 9.51;
    }
  } else if (CCDown) {
    CCSpeed = (int)VehicleVelocity;
    TargetRPM = CCSpeed * 9.51;
    CCactive = true;
    TargetCurrent = 0.6;
  }*/ 
  // Strömhållare:
  if (CCactive) { 
    if (CCDown && CCPower > 0) {
      CCPower = CCPower - 1;
      while (CCPower % 5 != 0) {
        CCPower--;
      }
      TargetCurrent = CCPower * 0.01;
    } else if (CCUp && CCPower < 100) {
      CCPower = CCPower + 1;
      while (CCPower % 5 != 0) {
        CCPower++;
      }
      TargetCurrent = CCPower * 0.01;
    }
  } else if (CCDown) {
    CCPower = Gas*100;
    TargetRPM = 20000;
    CCactive = true;
    TargetCurrent = CCPower * 0.01;
  }
  if (MaxGas - MinGas > 300 && MaxBrake - MinBrake > 300 && GearButton == 1) {  //ADD buttonpress for safety before entering driving
    InStartScreen = false;
  }
  LeftTurnButton = false; //La alla false efter, då alla ska vara false efter de lästs oavsett
  RightTurnButton = false;
  HazardButton = false;
  CCUp = false;
  CCDown = false;
  GearButton = false;
  
}

void DrivingLoop() {  //Loop som körs vid körning

  if (millis() > prevmsg + 100) {  //Meddelanden till motorn ska göras var 100ms
    sendDriveCAN(TargetRPM, TargetCurrent);
    prevmsg = millis();  //checkpoint för att skapa 100ms intervall
    sendESPCAN();
    twai_status_info_t status; // La till dessa, funkade inte alls utan dem men av någon anledning funkar det med dem utan att IF:en ens körs
    twai_get_status_info(&status);
   /* if (status.state == TWAI_STATE_BUS_OFF) {
      Serial.println("CAN Bus Off! Restarting...");
      twai_start();
    }*/
  }

  CanReadAll();  //Läs av och Uppdatera variabler, här behövs läggas till addresser för BMS och MPPT för att få tillgång till all info dem ger

  /*if (millis() > prevCons + 1000) {  //Denna måste finjusteras, siffrorna som bildas är nog inte helt 100
    UpdateConsum();
    prevCons = millis();
  }*/

}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void CanReadAll(){
  if (ESP32Can.readFrame(msg, 0)){
    switch (msg.identifier) {
      case 0x401:
        if (msg.data[2] != 0) {
          MCerror = true;
        } else {
          MCerror = false;
        }
        break;

      case 0x402: // Bus Measurement
        MotorCurrent = *((float*)(msg.data+4));  // De största 4 byten ger en nuvarande ström
        MotorVoltage = *((float*)(msg.data));    // De minsta 4 byten ger nuvarande spänning
        break;

      case 0x403: // Velocity Measurements
        VehicleVelocity = *((float*)(msg.data + 4)) * 3.6; // Hastighet i km/h
        MotorRPM = *((float*)(msg.data));
        break;

      case 0x40B: // Temperatur Measurements
        HeatSinkTemp = *((float*)(msg.data+4));
        MotorTemp = *((float*)(msg.data));
        break;

      case 0x40E: // Odometer & Bus AmpHours Measurement
        Amps = *((float*)(msg.data+4)); // Ah used since start
        Distance = *((float*)(msg.data)); // Distance travelled since start
        break;

      case 0x601: // BMS Data
        InternalTempBMS = msg.data[1];
        CellHighTemperature = msg.data[2];
        CellLowTemperature = msg.data[3];
        CellAverageTemperature = msg.data[4];
        CellHighTemperature_ID = msg.data[5];
        CellLowTemperature_ID = msg.data[6];
        if(msg.data[1] >=50 || msg.data[2] >=50 || msg.data[3] >=50 || msg.data[4] >=50 || msg.data[5] >=50 || msg.data[6]>=50){
          HighTemp = true;
        } else{
          HighTemp = false;
        }
        

        break;

      case 0x602: // Pack Data
        if (msg.data[0] >= 128) {
          PackCurrent = ((double)((msg.data[0] - 256) * 256 + msg.data[1])) / 10;
        } else {
          PackCurrent = ((double)(msg.data[0] * 256 + msg.data[1])) / 10;
        }
        PackVoltage = ((double)(msg.data[2] * 256 + msg.data[3])) / 10;
        LowCellVoltage = ((double)(msg.data[4] * 256 + msg.data[5])) / 10000;
        HighCellVoltage = ((double)(msg.data[6] * 256 + msg.data[7])) / 10000;
        break;

      case 0x603:
        AverageCellVoltage = ((double)(msg.data[0] * 256 + msg.data[1])) / 10000;
        break;

      case 0x605:
        if (msg.data[6] != 0 || msg.data[7] != 0) {
          BMSerror = true;
        } else {
          BMSerror = false;
        }
        break;
      case 0x200: // MPPT 1 Data
        MPPTInputVoltage1 = ((double)(msg.data[0] * 256 + msg.data[1])) / 100;
        if (msg.data[2] >= 128) {
          MPPTInputCurrent1 = ((double)((msg.data[2] - 256) * 256 + msg.data[3])) * 0.0005;
        } else {
          MPPTInputCurrent1 = ((double)(msg.data[2] * 256 + msg.data[3])) * 0.0005;
        }
        MPPTOutputVoltage1 = ((double)(msg.data[4] * 256 + msg.data[5])) / 100;
        if (msg.data[6] >= 128) {
          MPPTOutputCurrent1 = ((double)((msg.data[6] - 256) * 256 + msg.data[7])) * 0.0005;
        } else {
          MPPTOutputCurrent1 = ((double)(msg.data[6] * 256 + msg.data[7])) * 0.0005;
        }
        MPPTOutputPower1 = MPPTOutputVoltage1 * MPPTOutputCurrent1;
        break;

      case 0x175: // Message from ESP32 to turn off CC
        if(msg.data[0]!=0) {
          CCactive = false;
        }
        break;

      case 0x210: // MPPT 2 Data
        MPPTInputVoltage2 = ((double)(msg.data[0] * 256 + msg.data[1])) / 100;
        if (msg.data[2] >= 128) {
          MPPTInputCurrent2 = ((double)((msg.data[2] - 256) * 256 + msg.data[3])) * 0.0005;
        } else {
          MPPTInputCurrent2 = ((double)(msg.data[2] * 256 + msg.data[3])) * 0.0005;
        }
        MPPTOutputVoltage2 = ((double)(msg.data[4] * 256 + msg.data[5])) / 100;
        if (msg.data[6] >= 128) {
          MPPTOutputCurrent2 = ((double)((msg.data[6] - 256) * 256 + msg.data[7])) * 0.0005;
        } else {
          MPPTOutputCurrent2 = ((double)(msg.data[6] * 256 + msg.data[7])) * 0.0005;
        }
        MPPTOutputPower2 = MPPTOutputVoltage2 * MPPTOutputCurrent2;
        break;

      case 0x220: // MPPT 3 Data
        MPPTInputVoltage3 = ((double)(msg.data[0] * 256 + msg.data[1])) / 100;
        if (msg.data[2] >= 128) {
          MPPTInputCurrent3 = ((double)((msg.data[2] - 256) * 256 + msg.data[3])) * 0.0005;
        } else {
          MPPTInputCurrent3 = ((double)(msg.data[2] * 256 + msg.data[3])) * 0.0005;
        }
        MPPTOutputVoltage3 = ((double)(msg.data[4] * 256 + msg.data[5])) / 100;
        if (msg.data[6] >= 128) {
          MPPTOutputCurrent3 = ((double)((msg.data[6] - 256) * 256 + msg.data[7])) * 0.0005;
        } else {
          MPPTOutputCurrent3 = ((double)(msg.data[6] * 256 + msg.data[7])) * 0.0005;
        }
        MPPTOutputPower3 = MPPTOutputVoltage3 * MPPTOutputCurrent3;
        break;

      case 0x230: // MPPT 3 Data
        MPPTInputVoltage4 = ((double)(msg.data[0] * 256 + msg.data[1])) / 100;
        if (msg.data[2] >= 128) {
          MPPTInputCurrent4 = ((double)((msg.data[2] - 256) * 256 + msg.data[3])) * 0.0005;
        } else {
          MPPTInputCurrent4 = ((double)(msg.data[2] * 256 + msg.data[3])) * 0.0005;
        }
        MPPTOutputVoltage4 = ((double)(msg.data[4] * 256 + msg.data[5])) / 100;
        if (msg.data[6] >= 128) {
          MPPTOutputCurrent4 = ((double)((msg.data[6] - 256) * 256 + msg.data[7])) * 0.0005;
        } else {
          MPPTOutputCurrent4 = ((double)(msg.data[6] * 256 + msg.data[7])) * 0.0005;
        }
        MPPTOutputPower4 = MPPTOutputVoltage3 * MPPTOutputCurrent3;
        break;
    default:
      // Handle unknown CAN message IDs if needed
      break;
  }
  
  TotMPPT = MPPTOutputPower1 + MPPTOutputPower2 + MPPTOutputPower3 + MPPTOutputPower4;
  
  
  }
}

void sendDriveCAN(float value1, float value2) {
  uint8_t data[8];  // Array to hold 8 bytes (4 bytes per float)
  
  // Copy the float values into the data array
  memcpy(data, &value1, sizeof(float));       // Copy first float (4 bytes)
  memcpy(data + 4, &value2, sizeof(float));   // Copy second float (next 4 bytes)
  // Create and configure the CAN message
  twai_message_t msg1;
  msg1.identifier = 0x501;                    // Set CAN ID to 0x501 = Drive ID
  msg1.extd = 0;                               // Standard frame (0 for 11-bit)
  msg1.data_length_code = 8;                   // Message length (8 bytes)

  // Copy the data array into the CAN message data
  memcpy(msg1.data, data, 8);
  ESP32Can.writeFrame(msg1);
}

void sendESPCAN() {
  uint8_t data[8]={0,0,0,0,0,0,0,0};
  if(Left_Turn){
    data[0]=1;
  }else if(Right_Turn){
    data[1]=1;
  }else if(Hazard){
    data[2]=1;
  }
  if(Brake!=0){
    data[3]=1;
  }
  // Create and configure the CAN message
  twai_message_t msg1;
  msg1.identifier = 0x176;                    // Set CAN ID VIKTIGT FIXA RÄTT OCH KOLLA SÅ DE INTE ANVÄNDS AV NÅGOT ANNAT
  msg1.extd = 0;                               // Standard frame (0 for 11-bit)
  msg1.data_length_code = 8;                   // Message length (8 bytes)

  // Copy the data array into the CAN message data
  memcpy(msg1.data, data, 8);

  ESP32Can.writeFrame(msg1);
}

void UpdateConsum() {  //Uppdatera Nuvarande och snittförbrukning, Oklart om korrekta omvandlingar är gjorda måste testas. If satser för att förhindra division vid 0.

  if (Distance == 0) {  //omvandla till kWh/mil Ah*V/1000=kWh, 145 volt konstant för att få en jämförbar siffra
    AverageConsum = (Amps * 145 * 0.001) / (1 / 10000);
  } else {
    AverageConsum = (Amps * 145 * 0.001) / (Distance / 10000);
  }

  if (VehicleVelocity == 0) {  //Current A, vehicleVelocity= m/s A/(m/s)= As/m=Ah/(3600*m)   kWh/mil
    CurrentConsum = 0;
  } else {
    CurrentConsum = (MotorCurrent * MotorVoltage * 0.001) / (VehicleVelocity * 3.6 * 0.1);  // wh*0.001/km*0.1=kwh/mil
  }
}


















