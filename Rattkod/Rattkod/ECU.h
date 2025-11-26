#define GAS_POTENTIOMETER 16
#define BRAKE_POTENTIOMETER 15

#define LEFT_TURN_BUTTON 7
#define RIGHT_TURN_BUTTON 6
#define HAZARD_BUTTON 1
#define OK_GEAR_BUTTON 2
#define CC_UP_BUTTON 17
#define CC_DOWN_BUTTON 8
#define DRIVE_SELECT_BUTTON 9

void ReadButtons();
void CalibratePot();
void ReadPot();
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
void DrivingLoop();
void CruiseControl();
void CanReadAll();
void sendDriveCAN(float value1, float value2);
void sendESPCAN();
void UpdateConsum();
void ISR();