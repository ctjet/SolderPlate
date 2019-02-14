#include "mbed.h"
#include "st7735.h"
#include "menu.h"
#include "PID.h"
#include "mRotaryEncoder.h"
#include "splash.h"

 #define MAX 100
#define MIN 0

//ST7735_LCD lcd( PinName CS, PinName RESET, PinName RS, PinName SCL, PinName SDA, PinName BL = NC, backlight_t blType = Constant, float defaultBackLightLevel = 1.0 );
ST7735_LCD lcd( PB_12, PA_4, PB_1, PA_5, PA_7);
AnalogIn thr(PA_0);
AnalogIn pot(PA_1);

mRotaryEncoder adjust_enc(PB_14, PB_15, PA_3);//pinA,pinB,pinSW

float resolution = 1.0;
float adjust_set = 0.0;
float adjust_last = 0.0;
bool button = 1;


float AdjustVal(float adjust_val){
    adjust_val += (adjust_enc.Get() * resolution);    
    if( adjust_val >= MAX){ 
        adjust_val = MAX;
        }else if(adjust_val <= MIN){ 
            adjust_val = MIN;         
        }
        adjust_enc.Set(0);
    return adjust_val;
}
void Set_Resolution(void){
    button = !button;
    if( resolution == 1.0 ){
        resolution = 0.1;
        }else{
            resolution = 1.0;
            }
}           

DigitalOut myled(LED1);





//DigitalIn btn(PA_3);
DigitalOut relay(PB_3);

void doLineGraph(int xOffset = 5);
void drawSplash();
bool controlling = false;
uint16_t controllingWhich = 0;

uint16_t shortToRange(uint16_t val);

Timer timeSinceStart;


float thr_val =0, thr_res=0;

uint8_t potPos = 0;


//uint16_t keyFrames[100] = {0};

uint8_t numFrames = 6;
uint8_t curFrame = 0;

uint8_t frames[] = {0,125,150,215,215,0};
uint16_t times[] = {0,  30 ,150,180,185,300 };


float readThr(float &thr_val,float &thr_res ){
    	thr_val = (thr.read()*4095);
		thr_val = ((thr_val*3.3)/4095);
		thr_res = log(((3.3*(10/thr_val))-10)*1000);
		return ((1/(0.001129148+(0.000234125*thr_res)+(0.0000000876741*thr_res*thr_res*thr_res)))-273.15);
}



int main() {

    adjust_enc.attachSW( &Set_Resolution);



    //btn.mode(PullUp);
    potPos = pot.read_u16()/(65535/100 );

    lcd.Initialize();

    relay = 1;
    timeSinceStart.start();

    
    // print something on the screen
    //lcd.SetFont( &TerminusFont );
    
    drawSplash();
    
    while(1) {
        
        myled = 1;
        adjust_set = AdjustVal(adjust_set);
        adjust_last = adjust_set;


            
        //Print temp
        //char buf[10];
        //sprintf(buf, "%f", readThr(thr_val,thr_res));
        //lcd.Print( buf, CENTER, 25 ,-2,COLOR_BLACK); // align text to center horizontally and use starndard colors

        // if(btn){
        //     lcd.Print(" ", CENTER, 50 ,-2,COLOR_BLACK);
        // }else{
        //     lcd.Print("!", CENTER, 50 ,-2,COLOR_BLACK);
        // }
        potPos = pot.read_u16()/(65535/100 );
        doLineGraph();
        // lcd.Print(" ", potPos, 75 ,-2,COLOR_BLACK);
        
        
        // lcd.Print("^", pot.read_u16()/(65535/128 ), 75 ,COLOR_GREEN,COLOR_BLACK);
        //wait(0.1);
        
    }
}

void drawSplash(){
    lcd.FillRect( 0, 0, 127, 127, COLOR_WHITE );
    for(int i = 0; i<128; i++){
        for(int j = 0; j<128; j++){
            
            uint8_t byte = splashScreenImg[i][j/8];

            if  (((byte >> 7-(j%8)) & 1)==0){
                lcd.DrawPixel( j, i, COLOR_BLACK);
            }
            
            
        }
    }

    wait(5);
    lcd.ClearScreen();
}

float getCurVal(){
    int curPos = 0;
    long curTime = timeSinceStart.read();
    for(int i = 0; i<numFrames; i++){
        if(curTime>times[i]){
            curPos = i+1;
        }
    }
    if(curPos==0)
        curPos =1;
    if(curPos>=numFrames)
        curPos = numFrames-1;
    
    float rise = frames[curPos]-frames[curPos-1];
    float run = times[curPos]-times[curPos-1];
    return  (rise/run)*(curTime-times[curPos-1])+frames[curPos-1];
}

void doLineGraph(int xOffset){
    float curTemp = readThr(thr_val,thr_res);
    int xpos = (int)(timeSinceStart.read()/times[numFrames-1]*100)+xOffset;

    lcd.DrawLine(xpos-1,0,xpos-1,110,COLOR_BLACK);
    lcd.DrawLine(xpos,0,xpos,110,COLOR_GREEN);
    

    uint8_t maxTemp = 0;
    for(int i = 0; i<numFrames; i++){
        if(frames[i]> maxTemp)
            maxTemp = frames[i];
    }

    lcd.DrawPixel(xpos-1,110-(int)(curTemp*100/maxTemp),COLOR_RED);
    
    for(int i = 1; i<numFrames; i++){
        xpos = (int)(times[i]*100/times[numFrames-1])+xOffset;
        int prevX = (int)(times[i-1]*100/times[numFrames-1])+xOffset;
        int height = (int)(frames[i]*100/maxTemp);
        int prevHeight = (int)(frames[i-1]*100/maxTemp);
        lcd.DrawLine(xpos,110-height,prevX,110-prevHeight);
    }
    //Print temp
    char buf[10];
    sprintf(buf, "%.1f", curTemp);
    lcd.Print( buf, LEFT+3, 115 ,-2,COLOR_BLACK);


    sprintf(buf, "%.1f", getCurVal());
    lcd.Print( buf, LEFT + 90, 115 ,-2,COLOR_BLACK);


    sprintf(buf, "%.1f", adjust_set);
    lcd.Print( buf, LEFT + 40, 115 ,-2,COLOR_BLACK);
    // if(controlling){

    //     if(controllingWhich>=100&&controllingWhich<0){
    //         controllingWhich = 0;
    //     }
    //     uint16_t delta = potPos - keyFrames[controllingWhich];
    //     keyFrames[controllingWhich] = potPos;
        
    //     for(int i = controllingWhich+1; i<100; i++){
    //         keyFrames[i]+=delta;
    //         if(keyFrames[])
    //     }

    //                 lcd.DrawLine(5,110,5+1,110-keyFrames[0],controllingWhich==0?COLOR_GREEN:-2);
    //     for(int i = 1; i<100; i ++){
    //         lcd.DrawLine(i-1+5,110,i+5,110-keyFrames[i],controllingWhich==i?COLOR_GREEN:-2);

    //     }
    // }else{
    //         lcd.DrawLine(5,110,5+1,110-keyFrames[0],potPos==0?COLOR_RED:-2);
    //     for(int i = 1; i<100; i ++){
    //         lcd.DrawLine(i-1+5,110,i+5,110-keyFrames[i],potPos==i?COLOR_RED:-2);
    //     }
    //     if(btn){
    //         controlling = true;
    //         controllingWhich = potPos;
    //     }
    // }
}
 
