#include <Arduino.h>
#include <LCDHelper.h>

// IMPORTANT: LCDWIKI_KBV LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

// This program is a demo of displaying string
// Set the pins to the correct ones for your development shield or breakout board.
// when using the BREAKOUT BOARD only and using these 8 data lines to the LCD,
// pin usage as follow:
//              CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7
// Arduino Uno  A3  A2  A1  A0  A4   8   9   2   3   4   5   6   7
// Arduino Mega A3  A2  A1  A0  A4   8   9   2   3   4   5   6   7

// the 16bit mode only use in Mega.you must modify the mode in the file of lcd_mode.h
// when using the BREAKOUT BOARD only and using these 16 data lines to the LCD,
// pin usage as follow:
//              CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15
// Arduino Mega 40  38  39  44  41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29

// Remember to set the pins to suit your display module!

#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

// the definiens of 8bit mode as follow:
// if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV mylcd(ILI9341, A3, A2, A1, A0, A4); // model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9325,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9328,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8357D,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8347G,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8347I,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ST7735S,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset

// if the IC model is not known and the modules is readable,you can use this constructed function
// LCDWIKI_KBV mylcd(240,320,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(320,480,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset

// the definiens of 16bit mode as follow:
// if the IC model is known or the modules is unreadable,you can use this constructed function
// LCDWIKI_KBV mylcd(ILI9341,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9325,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9328,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8357D,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8347G,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(HX8347I,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9486,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9488,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ILI9481,40,38,39,44,41); //model,cs,cd,wr,rd,reset
// LCDWIKI_KBV mylcd(ST7735S,40,38,39,44,41); //model,cs,cd,wr,rd,reset

// if the IC model is not known and the modules is readable,you can use this constructed function
// LCDWIKI_KBV mylcd(240,320,40,38,39,44,41);//width,height,cs,cd,wr,rd,reset for
// LCDWIKI_KBV mylcd(320,480,40,38,39,44,41);//width,height,cs,cd,wr,rd,reset

#include <EEPROM.h>

#include <stdint.h>
#include "TouchScreen.h"

#define YP A2 // must be an analog pin, use "An" notation!
#define XM A1 // must be an analog pin, use "An" notation!
#define YM 6  // can be a digital pin
#define XP 7  // can be a digital pin

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint p;

LCDHelper::LCDHelper()
{
}

void LCDHelper::centerPrint(const char *s, int y)
{
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    int len = strlen(s) * 6;
    mylcd.Set_Text_colour(TFT_WHITE);
    mylcd.Print_String(s, (mylcd.Get_Width() - len) / 2, y - 40);
}

void LCDHelper::drawCrossHair(int y, int x, uint16_t color)
{
    mylcd.Set_Draw_color(color);
    mylcd.Draw_Rectangle(x - 9, y - 9, x + 9, y + 9);
    mylcd.Draw_Line(x - 5, y, x + 5, y);
    mylcd.Draw_Line(x, y - 5, x, y + 5);
}

void LCDHelper::calibrate()
{
    int x, y, cnt;
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    mylcd.Fill_Screen(TFT_BLACK);
    for (x = 10, cnt = 0; x < mylcd.Get_Width(); x += (mylcd.Get_Width() - 20) / 2)
    {
        for (y = 10; y < mylcd.Get_Height(); y += (mylcd.Get_Height() - 20) / 2)
        {
            if (++cnt != 5)
                drawCrossHair(x, y, TFT_BLUE);
        }
    }

    for (x = 10, cnt = 0; x < mylcd.Get_Width(); x += (mylcd.Get_Width() - 20) / 2)
    {
        for (y = 10; y < mylcd.Get_Height(); y += (mylcd.Get_Height() - 20) / 2)
        {
            if (++cnt != 5)
            {
                pinMode(XM, OUTPUT);
                pinMode(YP, OUTPUT);
                drawCrossHair(x, y, TFT_WHITE);
                centerPrint("*  PRESS  *", (mylcd.Get_Height() / 2) - 6);
                while (true)
                {
                    if (ts.pressure() < 600 && ts.pressure() > 400)
                    {

                        p = ts.getPoint();
                        centerPrint("*  HOLD!  *", (mylcd.Get_Height() / 2) - 6);
                        Serial.print("X = ");
                        Serial.print(p.x);
                        Serial.print("\tY = ");
                        Serial.println(p.y);

                        TouchPointX[0] = max(TouchPointX[0], p.x);
                        TouchPointX[1] = min(TouchPointX[1], p.x);
                        TouchPointY[0] = max(TouchPointY[0], p.y);
                        TouchPointY[1] = min(TouchPointY[1], p.y);

                        drawCrossHair(x, y, TFT_RED);
                        delay(100);
                        break;
                    }
                    delay(10);
                }
                centerPrint("* RELEASE *", (mylcd.Get_Height() / 2) - 6);
                // Serial.println("Wait");
                while (ts.pressure() != 0)
                {

                    delay(50);
                }
                p = ts.getPoint();
                pinMode(XM, OUTPUT);
                pinMode(YP, OUTPUT);
                drawCrossHair(x, y, TFT_YELLOW);
                delay(200);
            }
        }
    }

    delay(200);
    EEPROM.write(0, TouchPointX[0] & 0xFF);
    EEPROM.write(1, (TouchPointX[0] >> 8) & 0xFF);

    EEPROM.write(2, TouchPointX[1] & 0xFF);
    EEPROM.write(3, (TouchPointX[1] >> 8) & 0xFF);

    EEPROM.write(4, TouchPointY[0] & 0xFF);
    EEPROM.write(5, (TouchPointY[0] >> 8) & 0xFF);

    EEPROM.write(6, TouchPointY[1] & 0xFF);
    EEPROM.write(7, (TouchPointY[1] >> 8) & 0xFF);
}

// set current rotation
// 0  :  0 degree
// 1  :  90 degree
// 2  :  180 degree
// 3  :  270 degree
//
void LCDHelper::bootUp(byte Set_Rotation)
{
    Serial.print(mylcd.Get_Width());
    Serial.print(",");
    Serial.println(mylcd.Get_Height());

    mylcd.Init_LCD();
    mylcd.Set_Rotation(3);
    mylcd.Fill_Screen(TFT_BLACK);

    mylcd.Set_Text_Size(2);
    mylcd.Fill_Rect(0, 40, 150, 150, TFT_GREEN);
    mylcd.Set_Text_colour(TFT_BLACK);
    mylcd.Set_Text_Back_colour(TFT_GREEN);
    mylcd.Print_String("Calibration", 10, 110);

    mylcd.Fill_Rect(170, 40, 150, 150, TFT_RED);
    mylcd.Set_Text_colour(TFT_WHITE);
    mylcd.Set_Text_Back_colour(TFT_RED);

    mylcd.Print_String("Skip", 220, 110);
    while (true)
    {
        if (ts.pressure() < 600 && ts.pressure() > 400)
        {
            p = ts.getPoint();
            Serial.print("\t\t\tX = ");
            Serial.print(p.x);
            Serial.print("\tY = ");
            Serial.println(p.y);
            if (p.y <= 400)
                Calibration = false;
            else if (p.y >= 600)
                Calibration = true;
            break;
        }
    }

    if (Calibration)
        calibrate();
    else
    {
        TouchPointX[0] = EEPROM.read(0) + (EEPROM.read(1) << 8);
        TouchPointX[1] = EEPROM.read(2) + (EEPROM.read(3) << 8);
        TouchPointY[0] = EEPROM.read(4) + (EEPROM.read(5) << 8);
        TouchPointY[1] = EEPROM.read(6) + (EEPROM.read(7) << 8);
    }

    // 896-205------873-196
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    mylcd.Fill_Screen(TFT_BLACK);
    mylcd.Set_Draw_color(TFT_CYAN);
    drowColorBoxs();

    Serial.print(TouchPointX[0]);
    Serial.print('-');
    Serial.print(TouchPointX[1]);
    Serial.print("------");
    Serial.print(TouchPointY[0]);
    Serial.print('-');
    Serial.println(TouchPointY[1]);
    mylcd.Set_Rotation(Set_Rotation);
}
void LCDHelper::drowColorBoxs()
{
    BOXSIZE = mylcd.Get_Height() / 6;

    mylcd.Fill_Rect(0, 0, BOXSIZE, BOXSIZE, TFT_RED);
    mylcd.Fill_Rect(0, BOXSIZE, BOXSIZE, BOXSIZE, TFT_YELLOW);
    mylcd.Fill_Rect(0, BOXSIZE * 2, BOXSIZE, BOXSIZE, TFT_GREEN);
    mylcd.Fill_Rect(0, BOXSIZE * 3, BOXSIZE, BOXSIZE, TFT_CYAN);
    mylcd.Fill_Rect(0, BOXSIZE * 4, BOXSIZE, BOXSIZE, TFT_BLUE);
    mylcd.Fill_Rect(0, BOXSIZE * 5, BOXSIZE, BOXSIZE, TFT_MAGENTA);
}

void LCDHelper::drawPoint()
{
    p = ts.getPoint();
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!
    if (p.z > ts.pressureThreshhold)
    {
        switch (mylcd.Get_Rotation())
        {
        case 0:
            xpo = map(p.x, TouchPointX[1], TouchPointX[0], 0, mylcd.Get_Width());
            ypo = map(p.y, TouchPointY[1], TouchPointY[0], 0, mylcd.Get_Height());
            break;
        case 1:
            xpo = map(p.y, TouchPointX[1], TouchPointX[0], 0, mylcd.Get_Width());
            ypo = map(p.x, TouchPointY[0], TouchPointY[1], 0, mylcd.Get_Height());
            break;
        case 2:
            xpo = map(p.x, TouchPointX[0], TouchPointX[1], 0, mylcd.Get_Width());
            ypo = map(p.y, TouchPointY[0], TouchPointY[1], 0, mylcd.Get_Height());
            break;
        case 3:
            xpo = map(p.y, TouchPointX[0], TouchPointX[1], 0, mylcd.Get_Width());
            ypo = map(p.x, TouchPointY[1], TouchPointY[0], 0, mylcd.Get_Height());
            break;

        default:
            break;
        }

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        if (xpo > BOXSIZE)
            if (sizePoint > 0)
                mylcd.Fill_Circle(xpo, ypo, sizePoint);
            else
                mylcd.Draw_Pixel(xpo, ypo);
        else if (xpo < 0)
        {
            mylcd.Fill_Screen(TFT_BLACK);
            drowColorBoxs();
        }

        else
        {
            if (ypo < BOXSIZE)
                mylcd.Set_Draw_color(TFT_RED);
            else if (ypo < BOXSIZE * 2 && ypo > BOXSIZE)
                mylcd.Set_Draw_color(TFT_YELLOW);
            else if (ypo < BOXSIZE * 3 && ypo > BOXSIZE * 2)
                mylcd.Set_Draw_color(TFT_GREEN);
            else if (ypo < BOXSIZE * 4 && ypo > BOXSIZE * 3)
                mylcd.Set_Draw_color(TFT_CYAN);
            else if (ypo < BOXSIZE * 5 && ypo > BOXSIZE * 4)
                mylcd.Set_Draw_color(TFT_BLUE);
            else if (ypo < BOXSIZE * 6 && ypo > BOXSIZE * 5)
                mylcd.Set_Draw_color(TFT_MAGENTA);
        }
    }
}
// Config How Big the draw point
// select between 0 to 10
void LCDHelper::setSizePoint(byte size)
{
    sizePoint = size;
}