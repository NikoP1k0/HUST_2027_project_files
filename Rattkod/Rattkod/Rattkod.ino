#include "Platform.h"
#include "App_Common.h"
#include "DispFunctions.h"
#include "ECU.h"
#include <SPI.h>
#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX 4
#define CAN_RX 5

Gpu_Hal_Context_t host, *phost;

float MotorCurrent=0,MotorVoltage=0,VehicleVelocity=0,MotorRPM=0,DriverCurrent=0,DriverVelocity=0,Amps=0,Distance=0,HeatSinkTemp=0, MotorTemp=0;

uint8_t InternalTempBMS=0, CellHighTemperature=0, CellLowTemperature=0 ,CellAverageTemperature=0 ,CellHighTemperature_ID=0, CellLowTemperature_ID=0;

double PackCurrent=0, PackVoltage=0, LowCellVoltage=0, HighCellVoltage=0, AverageCellVoltage=0;

double MPPTInputVoltage1=0, MPPTInputCurrent1=0, MPPTOutputVoltage1=0, MPPTOutputCurrent1=0,MPPTOutputPower1=0;
double MPPTInputVoltage2=0, MPPTInputCurrent2=0, MPPTOutputVoltage2=0, MPPTOutputCurrent2=0,MPPTOutputPower2=0;
double MPPTInputVoltage3=0, MPPTInputCurrent3=0, MPPTOutputVoltage3=0, MPPTOutputCurrent3=0,MPPTOutputPower3=0;
double MPPTInputVoltage4=0, MPPTInputCurrent4=0, MPPTOutputVoltage4=0, MPPTOutputCurrent4=0,MPPTOutputPower4=0;
double TotMPPT=0;
float CurrentConsum=0,AverageConsum=0, currenttime=0;

float Gas=0, MaxGas=0, MinGas=5000;//Höjde till högre än 4096 (2^12) då det är högsta möjliga värdet
float Brake=0, MaxBrake=0, MinBrake=5000;
float TargetRPM=20000,TargetCurrent=0;
int GasLimiter=1;
int brakepedal=0;

int CCSpeed=0;
int CCPower=0;
int LightState=0;

unsigned long prevButton=0,prevmsg=0,prevCons=0,prevBlink=0;
bool CCUp=false, CCDown=false, LeftTurnButton=false, RightTurnButton=false, HazardButton=false, GearButton=false;
bool Left_Turn=false, Right_Turn=false, Hazard=false,SolarActive=false,SolarPrecharge=false;
bool DriveButton=false;
bool Drive=false;
bool BlinkState=false;
bool SafeState=false;
bool MCerror=true;
bool BMSerror=true;
bool InStartScreen = true, CCactive= false;
bool UpdButtonFlag=false;
bool HighTemp = false;// ska ändras till false

/* void IRAM_ATTR isr() {
  if(millis() > prevButton + 200){//Ändrade till 200, mest för att 170 störde mig
    UpdButtonFlag=true;
    prevButton = millis();
  }
  
} */
void setup(){

  Serial.begin(9600);
  delay(1000);
  //Init display
  phost = &host;
  Serial.println(GPIO_PD);
  App_Common_Init(&host);


  ESP32Can.setPins(CAN_TX, CAN_RX);
  ESP32Can.setRxQueueSize(10);
  ESP32Can.setTxQueueSize(10);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

  if(ESP32Can.begin()) {
    Serial.println("CAN bus started!");
  } else {
    Serial.println("CAN bus failed!");
  }

  if(ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
    Serial.println("CAN bus speed 500!");
  } else {
    Serial.println("CAN bus failed!");
  } 


  Serial.println("Startup done");
  PinSetup();
  Gas = analogRead(16);
  Brake = analogRead(15); 
  MinGas=Gas;
  MaxGas=Gas;
  MinBrake=Brake;
  MaxBrake=Brake;
}

void PinSetup(){
  //Inputs

  pinMode(GAS_POTENTIOMETER, INPUT); //Gas
  pinMode(BRAKE_POTENTIOMETER, INPUT); //Brake 
  pinMode(CC_UP_BUTTON, INPUT); //CCUp
  //attachInterrupt(11, isr, RISING);
  pinMode(CC_DOWN_BUTTON, INPUT); //CCDown
  //attachInterrupt(12, isr, RISING);
  pinMode(DRIVE_SELECT_BUTTON, INPUT); //DriveButton 
  pinMode(OK_GEAR_BUTTON, INPUT); //GearSelect
  //attachInterrupt(14, isr, RISING);
  pinMode(LEFT_TURN_BUTTON, INPUT); //LeftTurnButton
  //attachInterrupt(1, isr, RISING);
  pinMode(RIGHT_TURN_BUTTON, INPUT); //RightTurnButton
  //attachInterrupt(9, isr, RISING);
  pinMode(HAZARD_BUTTON, INPUT); //HazardButton
  //attachInterrupt(2, isr, RISING);

  Serial.println("Arduino pinsetup done");

}
void loop(){
  if(millis() > prevButton + 100){//Ändrade till 200, mest för att 170 störde mig
    prevButton = millis();
    ReadButtons();
    
  }
 //Uppdatera Gas & Broms
/*   if (UpdButtonFlag) {  //Kolla knapptryck om interrupt triggats
    ReadButtons();
    //CruiseControl();
    UpdButtonFlag=false;
  }  */
   if(InStartScreen){//Flagga som indikerar vilken skärm som visas
    start_screen();//Skärm där vi ställer potentiometrar
    CalibratePot();
  }else{
    ReadPot();
    main_screen();//info som hastighet etc
    DrivingLoop();//Can och andra viktigheter
  } 


}

void start_screen() {
  
  char maxgas_char[20];
  char mingas_char[20];
  char maxbrake_char[20];
  char minbrake_char[20];

  sprintf(maxgas_char, "Max:%d", (int)MaxGas);
  sprintf(mingas_char, "Min:%d", (int)MinGas);
  sprintf(maxbrake_char, "Max:%d", (int)MaxBrake);
  sprintf(minbrake_char, "Min:%d", (int)MinBrake);

  Start_Set_Display(phost);

  // ---- Set boxes for text --------- //
  draw_rect(phost, 10, 60, 200, 100, 70, 200, 200);
  draw_rect(phost, 280, 60, 470, 100, 70, 200, 200);
  draw_rect(phost, 10, 130, 200, 170, 70, 200, 200);
  draw_rect(phost, 280, 130, 470, 170, 70, 200, 200);
  //draw_rect(phost, 140, 200, 340, 230, 70, 200, 200);

  //--------- INPUT text to screen ------------- //
  Write_Text(phost, 178, 0, 30, "HUST");
  Write_Text(phost, 360, 10, 25, "Gas");
  Write_Text(phost, 50, 10, 25, "Brake");
  
  Write_Text(phost, 10, 65, 25, maxbrake_char);
  Write_Text(phost, 10, 135, 25, minbrake_char);
  Write_Text(phost, 280, 65, 25, maxgas_char);
  Write_Text(phost, 280, 135, 25, mingas_char);

  Write_Text_Color(phost, 135, 215, 16, "SET GAS &  BRAKE", 255, 1, 1);




  Finish_Display(phost);
}


void main_screen() {//Skärm som visas under körning, fixa så att allting syns och ligger på bra position till, alla färdiga funktioner hittas i DispFunctions
  Start_Set_Display(phost);

  char gas_char[10];
  char brake_char[20];

  char TCurrent_char[20];
  char TRPM_char[20];

  char MinGas_char[20];
  char MaxGas_char[20];
  char MinBrake_char[20];
  char MaxBrake_char[20];

  char Solar_Power_char[20];
  char Motor_Power_char[20];
  char Battery_Power_char[20];

  char High_Voltage_char[20];
  char Avg_Voltage_char[20];
  char Low_Voltage_char[20];
  char Pack_Voltage_char[20];

  char MC_Temp_char[20];
  char Motor_Temp_char[20];
  char CellHigh_Temp_Char[20];
  char CellLow_Temp_Char[20];
  char BMS_Temp_Char[20];
  
  char velocity_char[10];

  //Allt kommenterat är för att läsa all information, ta inte bort!

  sprintf(gas_char, "Gas: %.2f", Gas);
  sprintf(brake_char, "Brake: %.2f", Brake);

  sprintf(TCurrent_char, "Current %.2f", TargetCurrent);
  sprintf(TRPM_char, "RPM %.2f", TargetRPM);

  sprintf(MinGas_char, "MinGas %.2f", MinGas);
  sprintf(MaxGas_char, "MaxGas %.2f", MaxGas);  
  sprintf(MinBrake_char, "MinBrake %.2f", MinBrake);
  sprintf(MaxBrake_char, "MaxBrake %.2f", MaxBrake);

  sprintf(Solar_Power_char, "Solar:%.1fW", TotMPPT);
  sprintf(Motor_Power_char, "Motor:%.1fW", -1*MotorCurrent*MotorVoltage);
  sprintf(Battery_Power_char, "Battery:%.1fW", -1*PackCurrent*PackVoltage);

  sprintf(High_Voltage_char, "High: %.3fV", HighCellVoltage);
  sprintf(Avg_Voltage_char, "Avg: %.3fV", AverageCellVoltage);
  sprintf(Low_Voltage_char, "Low: %.3fV", LowCellVoltage);
  sprintf(Pack_Voltage_char, "Pack: %.1fV", PackVoltage);

  
  sprintf(MC_Temp_char, "Mc: %d", (int)HeatSinkTemp);
  sprintf(Motor_Temp_char, "Motor:%d", (int)MotorTemp);
  sprintf(CellHigh_Temp_Char, "CellHi: %d", CellHighTemperature);
  sprintf(CellLow_Temp_Char, "CellLo: %d", CellLowTemperature);
  sprintf(BMS_Temp_Char, "Bms: %d", InternalTempBMS);
  

  sprintf(velocity_char, "%dkm/h", (int) VehicleVelocity);


  Write_Text(phost, 0, 100, 20, gas_char); 
  Write_Text(phost, 0, 110, 20, brake_char); 

  Write_Text(phost, 0, 120, 20, TCurrent_char); 
  Write_Text(phost, 0, 130, 20, TRPM_char); 
  
  Write_Text(phost, 0, 160, 20, MinGas_char); 
  Write_Text(phost, 0, 170, 20, MaxGas_char); 

  Write_Text(phost, 0, 180, 20, MinBrake_char); 
  Write_Text(phost, 0, 190, 20, MaxBrake_char); 



  insert_line(phost, 0, 150, 0, 80, 20);
  Write_Text(phost, 40, 0, 21, "Power");
  Write_Text(phost, 10, 20, 20, Motor_Power_char); 
  Write_Text(phost, 10, 30, 20, Solar_Power_char);
  Write_Text(phost, 10, 40, 20, Battery_Power_char);
  

  insert_line(phost,150,300,0,80,20);
  Write_Text(phost,180, 0, 21, "Voltages");
  Write_Text(phost, 170, 20, 20, High_Voltage_char); 
  Write_Text(phost, 170, 30, 20, Avg_Voltage_char); 
  Write_Text(phost, 170, 40, 20, Low_Voltage_char); 
  Write_Text(phost, 170, 50, 20, Pack_Voltage_char); 

  insert_line(phost,300,500,0,80,20);
  Write_Text(phost,320, 0, 22, "Temperatures");
  Write_Text(phost, 330, 20, 20, MC_Temp_char); 
  Write_Text(phost, 330, 30, 20, Motor_Temp_char); 
  Write_Text(phost, 330, 40, 20, CellHigh_Temp_Char); 
  Write_Text(phost, 330, 50, 20, CellLow_Temp_Char);
  Write_Text(phost, 330, 60, 20, BMS_Temp_Char);
   

  Write_Text(phost, 200, 200, 24, velocity_char);


//Nedan är layout för körning
/*

  sprintf(Solar_Power_char, "Solar:%dW", (int)TotMPPT);
  sprintf(Motor_Power_char, "Motor:%dW", (int)(-1 * MotorCurrent * MotorVoltage));
  sprintf(Battery_Power_char, "Battery:%dW", (int)(-1 * PackCurrent * PackVoltage));

  sprintf(Low_Voltage_char, "Low: %.3fV", LowCellVoltage);
  sprintf(Pack_Voltage_char, "Pack: %.1fV", PackVoltage);


  sprintf(velocity_char, "%dkm/h", (int)VehicleVelocity);

  insert_line(phost, 0, 500, 100, 100, 18);
  Write_Text(phost, 20, 0, 23, "Power");
  Write_Text(phost, 10, 25, 22, Motor_Power_char);
  Write_Text(phost, 10, 50, 22, Solar_Power_char);
  Write_Text(phost, 10, 75, 22, Battery_Power_char);

  insert_line(phost, 320, 320, 0, 100, 18);
  insert_line(phost, 150, 150, 0, 100, 18);
  Write_Text(phost, 330, 0, 23, "Voltages");
  Write_Text(phost, 325, 25, 22, Low_Voltage_char);
  Write_Text(phost, 335, 50, 22, Pack_Voltage_char);


  Write_Text(phost, 170, 30, 29, velocity_char);

  */
//Allt ner hit är layout för körning

  if(CCactive){
    cruise_control_icon(phost,160,170,CCPower);//
  }
  
  driving_mode_icon(phost,0,0,0,DriveButton);

  if(DriveButton){
    ECO_mode_icon(phost,0,0,0,GasLimiter);
  }

  if(BMSerror){
    volt_battery_icon(phost,180,120,255,0,0); 
  }
  
  if(MCerror){
    mc_icon(phost,250,120,255,255,0);
  }
  
  if (HighTemp) {
    hightemp_battery_icon(phost, 110, 115, 255, 0, 0);
  }


  if(Hazard){
    TurnL_icon(phost,40,200);
    TurnR_icon(phost,410,200);
  }else if(Left_Turn){
    TurnL_icon(phost,40,200);
  }else if(Right_Turn){
    TurnR_icon(phost,410,200);
  } 

  Finish_Display(phost);
}