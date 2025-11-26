#include <SPI.h>
#include "Platform.h"
#include "App_Common.h"


/* Function for writing a text on the screen */
/*void Write_Text(Gpu_Hal_Context_t *phost, int x, int y, int size, char text[])
{
  App_WrDl_Buffer(phost, BEGIN(BITMAPS));
  App_WrDl_Buffer(phost, COLOR_RGB(1, 1, 1));
  int increment = 8;

  for(int i = 0; text[i] != '\0'; i++)
  {
    
    App_WrDl_Buffer(phost, VERTEX2II(x, y, size, text[i]));

    
    if(text[i] == 'W' || text[i] == 'M'){
      x += size*size/15;
    } else  if(text[i] == 'm') {
      x += size*size*size/(24*24);
    } else { 
      x += size*size/24;
    }
  }
  App_WrDl_Buffer(phost, END());
}*/
void Write_Text(Gpu_Hal_Context_t *phost, int x, int y, int size, char text[]) {
    App_WrDl_Buffer(phost, BEGIN(BITMAPS));
    App_WrDl_Buffer(phost, COLOR_RGB(1, 1, 1));

    int baseIncrement,capitalWideIncrement, wideCharIncrement, narrowCharIncrement;

    if (size <= 22) {  
        // Tighter spacing for smaller sizes
        baseIncrement = size * size /40;
        capitalWideIncrement=size * size / 25;
        wideCharIncrement = size * size / 30;
        narrowCharIncrement = size * size / 50;
    } else {
        // Larger spacing for larger sizes
        baseIncrement = size * size / 28;        
        capitalWideIncrement = size * size / 22;
        wideCharIncrement = size * size / 25;
        narrowCharIncrement = size * size / 40;
    }

    for(int i = 0; text[i] != '\0'; i++) {
        App_WrDl_Buffer(phost, VERTEX2II(x, y, size, text[i]));

        // Adjust the x position based on the character width
        if(text[i] == 'W' || text[i] == 'M') {
            x += capitalWideIncrement;
        }else if(text[i] == 'w' || text[i] == 'm'||isUpperCase(text[i])){
            x += wideCharIncrement;
        }else if(text[i] == 'i' || text[i] == 'l' || text[i] == '1' || text[i] == 't' || text[i] == 'I'||text[i] == 'r'||text[i] == '/') {
            x += narrowCharIncrement;
        } else {
            x += baseIncrement;
        }
    }

    App_WrDl_Buffer(phost, END());
}



/* Function for writing a text on the screen */
void Write_Text_Color(Gpu_Hal_Context_t *phost, int x, int y, int size, char text[], int r, int g, int b)
{
  App_WrDl_Buffer(phost, BEGIN(BITMAPS));
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  int increment = 8;

  for(int i = 0; text[i] != '\0'; i++)
  {
    
    App_WrDl_Buffer(phost, VERTEX2II(x, y, size, text[i]));

    if(isUpperCase(text[i])){
      x += size;
    } else if(text[i] == 'r' || text[i] == 'i') {
      x += size - size/3*1.5;
    } else  if(text[i] == 'm') {
      x += size + 5;
    } else { 
      x += size - increment;
    }
  }
  App_WrDl_Buffer(phost, END());
}

/* Starts a display list */
void Start_Set_Display(Gpu_Hal_Context_t *phost)
{
  App_WrDl_Buffer(phost, CLEAR_COLOR_RGB(255, 255, 255));
  App_WrDl_Buffer(phost, CLEAR(1, 1, 1));
  ///App_WrDl_Buffer(phost, BEGIN(BITMAPS));
}

/* Swaps the disply list */
void Finish_Display(Gpu_Hal_Context_t *phost) //void vehicleVelocityFinish_Display(Gpu_Hal_Context_t *phost)
{
    App_WrDl_Buffer(phost, END());
    App_WrDl_Buffer(phost, COLOR_RGB(255, 1, 1));
    App_WrDl_Buffer(phost, DISPLAY());
        
    Gpu_Hal_DLSwap(phost,DLSWAP_FRAME);
    App_Flush_DL_Buffer(phost);    
}


void insert_single_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth, int r, int g, int b) {
  App_WrDl_Buffer(phost, BEGIN(LINES));
  App_WrDl_Buffer(phost, LINE_WIDTH(linewidth));
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));  
  App_WrDl_Buffer(phost, END());
}


/* Creates a box on the screen */
void insert_line(Gpu_Hal_Context_t *phost, int x1, 
                      int x2, int y1, int y2, int linewidth) 
  {
  App_WrDl_Buffer(phost, BEGIN(LINES));
  App_WrDl_Buffer(phost, LINE_WIDTH(linewidth));
  App_WrDl_Buffer(phost, COLOR_RGB(10, 50, 10));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y2, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y2, 0, 0));

  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));


  
  App_WrDl_Buffer(phost, END());
}


void insert_scissor(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int r, int g, int b) {
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  App_WrDl_Buffer(phost, SCISSOR_XY(x1, y1));
  App_WrDl_Buffer(phost, SCISSOR_SIZE(x2, y2));
}


/* Logic for the charging on the screen */
void insert_charging(Gpu_Hal_Context_t *phost, float charging) {

  App_WrDl_Buffer(phost, BEGIN(POINTS));
  App_WrDl_Buffer(phost, POINT_SIZE(150));

  if(charging > 0.9) {
    App_WrDl_Buffer(phost, COLOR_RGB(1, 250, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 25, 0, 0));
  }

  if(charging > 0.8){
    App_WrDl_Buffer(phost, COLOR_RGB(10, 200, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 50, 0, 0));
  }
  
  if(charging > 0.7){
    App_WrDl_Buffer(phost, COLOR_RGB(20, 150, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 75, 0, 0));
  }
  
  if(charging > 0.6){  
    App_WrDl_Buffer(phost, COLOR_RGB(70, 150, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 100, 0, 0));
  }

  if(charging > 0.5) {
    App_WrDl_Buffer(phost, COLOR_RGB(150, 180, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 125, 0, 0));
  }

  if(charging > 0.4) {
    App_WrDl_Buffer(phost, COLOR_RGB(180, 150, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 150, 0, 0));
  }

  if(charging > 0.3) {
    App_WrDl_Buffer(phost, COLOR_RGB(200, 100, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 175, 0, 0));
  }

  if(charging > 0.2){
    App_WrDl_Buffer(phost, COLOR_RGB(200, 50, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 200, 0, 0));
  }

  if(charging > 0.1)  {
    App_WrDl_Buffer(phost, COLOR_RGB(240, 50, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 225, 0, 0));
  }

  if(charging > 0.5){
    App_WrDl_Buffer(phost, COLOR_RGB(255, 1, 1));
    App_WrDl_Buffer(phost, VERTEX2II(455, 250, 0, 0));
  }
  
  App_WrDl_Buffer(phost, END());
}


void draw_rect(Gpu_Hal_Context_t *phost, int x1, int y1, int x2, int y2, int r, int g, int b) {
  App_WrDl_Buffer(phost, BEGIN(RECTS));
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  App_WrDl_Buffer(phost, LINE_WIDTH(100));
  
  /* Using * 16 becuase input to VERTEX2F is 1/16 pixel*/    
  App_WrDl_Buffer(phost, VERTEX2F(x1 * 16, y1 * 16));
  App_WrDl_Buffer(phost, VERTEX2F(x2 * 16, y2 * 16));

  App_WrDl_Buffer(phost, END());
}


void driving_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, bool mode) {

  char letter[1];
  if(mode){
    sprintf(letter, "D");
  }else{
    sprintf(letter, "N");
  }

  Write_Text_Color(phost, 225, 230, 31, letter,0,0,0);
}

void ECO_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, int mode) {

  char number[1];
  sprintf(number,"%d", mode);
  Write_Text_Color(phost, 200, 230, 31, number,0,0,0);
}

void volt_battery_icon(Gpu_Hal_Context_t *phost,int x, int y, int r, int g, int b) {
  draw_rect(phost, x, y, x+40, y+30, r, g, b);
  insert_line(phost, x-5, x+45, y-5, y+35, 20);
  insert_single_line(phost,x+5,x+15,y+15,y+15,20,0,0,0);
  insert_single_line(phost,x+25,x+35,y+15,y+15,20,0,0,0);
  insert_single_line(phost,x+30,x+30,y+10,y+20,20,0,0,0);

  //insert_single_line(phost,x+5,x+15,y-5,y-10,20,0,0,0);
  insert_line(phost,x+5,x+15,y-5,y-10,20);
  insert_line(phost,x+25,x+35,y-5,y-10,20);
  
}


void mc_icon(Gpu_Hal_Context_t *phost,int x,int y, int r, int g, int b) {
  draw_rect(phost, x, y, x+40, y+30, r, g, b);
  Write_Text(phost, x+5, y+5, 22, "MC");
  insert_line(phost, x-5, x+45, y-5, y+35, 20);
}

void hightemp_battery_icon(Gpu_Hal_Context_t *phost,int x, int y, int r, int g, int b) {
  insert_single_line(phost,x+28,x+34,y+10,y+10,18,255,0,0);//1
  insert_single_line(phost,x+28,x+34,y+15,y+15,18,255,0,0);//2
  insert_single_line(phost,x+28,x+34,y+20,y+20,18,255,0,0);//3
  insert_single_line(phost,x+5,x+17,y+29,y+29,24,255,0,0);//4
  insert_single_line(phost,x+22,x+29,y+29,y+29,26,255,0,0);//5
  insert_single_line(phost,x+34,x+46,y+29,y+29,24,255,0,0);//6
  insert_single_line(phost,x+7,x+44,y+36,y+36,24,255,0,0);//7
  insert_single_line(phost,x+26,x+26,y+6,y+29,24,255,0,0);//8
  insert_single_line(phost,x+7,x+11,y+27,y+27,14,255,0,0);//9
  insert_single_line(phost,x+16,x+17,y+27,y+27,14,255,0,0);//10
  insert_single_line(phost,x+34,x+35,y+27,y+27,14,255,0,0);//11
  insert_single_line(phost,x+41,x+44,y+27,y+27,14,255,0,0);//12
  insert_single_line(phost,x+5,x+6,y+31,y+31,14,255,0,0);//13
  insert_single_line(phost,x+12,x+16,y+31,y+31,14,255,0,0);//14
  insert_single_line(phost,x+36,x+40,y+31,y+31,14,255,0,0);//15
  insert_single_line(phost,x+45,x+46,y+31,y+31,14,255,0,0);//16
  insert_single_line(phost,x+11,x+14,y+34,y+34,14,255,0,0);//17
  insert_single_line(phost,x+18,x+21,y+34,y+34,14,255,0,0);//18
  insert_single_line(phost,x+25,x+28,y+34,y+34,14,255,0,0);//19
  insert_single_line(phost,x+34,x+37,y+34,y+34,14,255,0,0);//20
  insert_single_line(phost,x+42,x+44,y+34,y+34,14,255,0,0);//21
  insert_single_line(phost,x+7,x+9,y+38,y+38,14,255,0,0);//22
  insert_single_line(phost,x+15,x+17,y+38,y+38,14,255,0,0);//23
  insert_single_line(phost,x+22,x+24,y+38,y+38,14,255,0,0);//24
  insert_single_line(phost,x+29,x+33,y+38,y+38,14,255,0,0);//25
  insert_single_line(phost,x+38,x+41,y+38,y+38,14,255,0,0);//26
}


void solar_cell_icon(Gpu_Hal_Context_t *phost,int x, int y, bool PCstate,bool SolActive) {
  int r=0;
  int g=0;
  int b=0;
  if(SolActive){
    b=255;
  }else if(PCstate){
    r=200;
    g=200;
    b=200;
  }else{
    return;
  }
  draw_rect(phost, x, y, x+55, y+40, r, g, b);
  /* Make a grid in rect*/

  insert_line(phost, x, x+55, y, y+40, 20);

  insert_line(phost, x+8, x+18, y+6, y+16, 20);
  insert_line(phost, x+23, x+33, y+6, y+16, 20);
  insert_line(phost, x+38, x+48, y+6, y+16, 20);

  insert_line(phost, x+8, x+18, y+23, y+33, 20);
  insert_line(phost, x+23, x+33, y+23, y+33, 20);
  insert_line(phost, x+38, x+48, y+23, y+33, 20);
}


void error_icon(Gpu_Hal_Context_t *phost, int r, int g, int b) {
  draw_rect(phost, 87, 220, 188, 260, r, g, b);
  Write_Text(phost, 85, 230, 23, "ERROR");
  insert_line(phost, 82, 193, 215, 265, 20);
}


void meter_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage) {
  insert_line(phost, x1, x2, y1, y2, 25);
  float counter = 0.9;  
 
  for(int i = y1 + 15; i <= y2-10; i += (y2-y1)/10) {
    if(counter < percentage) {
      draw_rect(phost, x1+10, i, x2-10, i+2, 1, 1, 1);
    }  
    counter -= 0.1;
  }
}

void current_consume_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage){
  draw_rect(phost, x1, y1, x2, y2, 0, 0, 0);
  draw_rect(phost, x1, y1+2, (x1+x2)/2 +((x1+x2)/2*percentage)-2, y2-2, 0, 255, 0);
  //draw_rect(phost, x1+5, x2-(x2*percentage), y1+2, y2-2, 0, 1, 0);
  //insert_line(phost, x1, x2, y1, y2, 20);
  
}

void economy_icon(Gpu_Hal_Context_t *phost, int meter_status) {
  draw_rect(phost, 177, 13, 191, 56, 255, 1, 1);
  draw_rect(phost, 204, 13, 219, 56, 255, 200, 1);
  draw_rect(phost, 232, 13, 248, 56, 255, 255, 1);
  draw_rect(phost, 261, 13, 276, 56, 200, 230, 1);
  draw_rect(phost, 289, 13, 303, 56, 100, 255, 1);
  insert_single_line(phost, 240, 240, 10, 60, 40, 10, 50 ,10);
  insert_line(phost, 170, 310, 8, 60, 40);
  //insert_scissor(phost, 172, 10, 308, 58, 200, 1, 1);
}


void drawHeadlight(Gpu_Hal_Context_t *phost,int State, int x, int y) {//120 150

  if(State==2){
    App_WrDl_Buffer(phost, BEGIN(BITMAPS));
    App_WrDl_Buffer(phost, COLOR_RGB(20, 20, 230));
    App_WrDl_Buffer(phost, VERTEX2II(x, y, 31, 'D'));
    App_WrDl_Buffer(phost, END());

    insert_single_line(phost, x-20, x-2, y+12, y+12, 40, 15, 20, 250);
    insert_single_line(phost, x-20, x-2, y+18, y+18, 40, 15, 20, 250);
    insert_single_line(phost, x-20, x-2, y+24, y+24, 40, 15, 20, 250);
    insert_single_line(phost, x-20, x-2, y+30, y+30, 40, 15, 20, 250);
    insert_single_line(phost, x-20, x-2, y+36, y+36, 40, 15, 20, 250);
  }else if(State==1){
    App_WrDl_Buffer(phost, BEGIN(BITMAPS));
    App_WrDl_Buffer(phost, COLOR_RGB(0, 150, 0));
    App_WrDl_Buffer(phost, VERTEX2II(x, y, 31, 'D'));
    App_WrDl_Buffer(phost, END());

    insert_single_line(phost, x-20, x-2, y+16, y+12, 40, 0, 150, 0);
    insert_single_line(phost, x-20, x-2, y+22, y+18, 40, 0, 150, 0);
    insert_single_line(phost, x-20, x-2, y+28, y+24, 40, 0, 150, 0);
    insert_single_line(phost, x-20, x-2, y+34, y+30, 40, 0, 150, 0);
    insert_single_line(phost, x-20, x-2, y+40, y+36, 40, 0, 150, 0);
  }else{
    App_WrDl_Buffer(phost, BEGIN(BITMAPS));
    App_WrDl_Buffer(phost, COLOR_RGB(200, 200, 200));
    App_WrDl_Buffer(phost, VERTEX2II(x, y, 31, 'D'));
    App_WrDl_Buffer(phost, END());

    insert_single_line(phost, x-20, x-2, y+16, y+12, 40, 200, 200, 200);
    insert_single_line(phost, x-20, x-2, y+22, y+18, 40, 200, 200, 200);
    insert_single_line(phost, x-20, x-2, y+28, y+24, 40, 200, 200, 200);
    insert_single_line(phost, x-20, x-2, y+34, y+30, 40, 200, 200, 200);
    insert_single_line(phost, x-20, x-2, y+40, y+36, 40, 200, 200, 200);
  }
}


void TurnL_icon(Gpu_Hal_Context_t *phost, int x, int y){

  draw_rect(phost,x,y,x+30,y+30,0,255,0);
  insert_single_line(phost,x-5,x-5,y-10,y+40,50,0,255,0);
  insert_single_line(phost,x-30,x-5,y+15,y-10,50,0,255,0);
  insert_single_line(phost,x-30,x-5,y+15,y+40,50,0,255,0);
  insert_single_line(phost,x-10,x-10,y+5,y+25,100,0,255,0);
  insert_single_line(phost,x-20,x-10,y+15,y+15,125,0,255,0);
  
}

void TurnR_icon(Gpu_Hal_Context_t *phost, int x, int y){

  draw_rect(phost,x,y,x+30,y+30,0,255,0);
  insert_single_line(phost,x+35,x+35,y-10,y+40,50,0,255,0);
  insert_single_line(phost,x+35,x+60,y-10,y+15,50,0,255,0);
  insert_single_line(phost,x+35,x+60,y+40,y+15,50,0,255,0);
  insert_single_line(phost,x+40,x+40,y+5,y+25,100,0,255,0);
  insert_single_line(phost,x+40,x+50,y+15,y+15,125,0,255,0);

  
}

void cruise_control_icon(Gpu_Hal_Context_t *phost,int x, int y, float cruise_velocity) {
  char cruise[20];
  sprintf(cruise, "CC:%d %", (int)cruise_velocity);
  
  Write_Text(phost, x, y, 22, cruise);
}






