#include <SPI.h>
#include "Platform.h"
#include "App_Common.h"

void Flash_Light(int delay, int pin);
void Write_Text(Gpu_Hal_Context_t *phost,  int x, int y, int size, char text[]);
void Write_Text_Color(Gpu_Hal_Context_t *phost, int x, int y, int size, char text[], int r, int g, int b);
void Finish_Display(Gpu_Hal_Context_t *phost);
void Start_Set_Display(Gpu_Hal_Context_t *phost);
void insert_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth);
void insert_charging(Gpu_Hal_Context_t *phost, float charging); 
void draw_rect(Gpu_Hal_Context_t *phost, int x1, int y1, int x2, int y2, int r, int g, int b);
void driving_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, bool mode);
void ECO_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, int mode);
void hightemp_battery_icon(Gpu_Hal_Context_t *phost,int x,int y, int r, int g, int b);
void volt_battery_icon(Gpu_Hal_Context_t *phost,int x,int y, int r, int g, int b);
void mc_icon(Gpu_Hal_Context_t *phost,int x,int y, int r, int g, int b);
void solar_cell_icon(Gpu_Hal_Context_t *phost,int x, int y, bool PCstate,bool SolActive);
void error_icon(Gpu_Hal_Context_t *phost, int r, int g, int b);
void meter_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage);
void current_consume_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage);
void economy_icon(Gpu_Hal_Context_t *phost, int meter_status);
void insert_scissor(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int r, int g, int b);
void insert_single_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth, int r, int g, int b);
void drawHeadlight(Gpu_Hal_Context_t *phost,int State, int x, int y);
void high_voltage(Gpu_Hal_Context_t *phost);
void TurnL_icon(Gpu_Hal_Context_t *phost, int x, int y);
void TurnR_icon(Gpu_Hal_Context_t *phost, int x, int y);
void cruise_control_icon(Gpu_Hal_Context_t *phost,int x, int y, float cruise_velocity);
