#include "Intro.h"

#include <avr/pgmspace.h>

#include "Arduboy.h"

constexpr unsigned char arduino[] PROGMEM
{
  0x3F, 0xFF, 0xFF, 0xFC, 0x40, 0x00, 0x00, 0x02, 0x89, 0x99, 0x54, 0x91, 0x95, 0x55, 0x56, 0xA9,
0x9D, 0x95, 0x55, 0xA9, 0x95, 0x59, 0xD4, 0x91, 0x40, 0x00, 0x00, 0x02, 0x3F, 0xFF, 0xFF, 0xFC
};

void showIntro()    // Show Arduino retro intro
{
  for(int i = -8; i < 28; i += 2)
  {
    arduboy.clear();
    arduboy.drawSlowXYBitmap(46, i, arduino, 32, 8, 1);
    updateDisplay();
    delay(1000/30);
  }

  #if SOUND == ON
    sound.tone(987, 160);
    delay(160);

    sound.tone(1318, 400);
    delay(2000);
  #endif
}