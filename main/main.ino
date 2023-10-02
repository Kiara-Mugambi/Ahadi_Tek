#include <Arduino.h>
//#include <LCDHelper.h>

// IMPORTANT: LCDWIKI_KBV LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// GO TO LCDHelper.h AND CONFIG YOUR LCD AND TouchScreen
// REMEMBER TO CHECK YOUR PINOUT FOR LCD AND TouchScreen
// THIS CODE CALIBRATES THE TouchScreen AUTOMATICCALLY AND SAVES IT ON EEPROM. AFTER THAT YOU CAN SELECT SKIP IN BOOT UP
// LCD CONFIG IS IN LINES  26 - 56
// TouchScreen CONFIG IS IN LINES 63-66
LCDHelper lcdhelper;

void setup()
{
  Serial.begin(9600);
  lcdhelper.bootUp(3);
  lcdhelper.setSizePoint(1);
}

void loop()
{
  lcdhelper.drawPoint();
}
