#include "Ending.h"

#include "Settings.h"
#include "Arduboy.h"

#if FX_MODE == ON
#include "fxdata.h"
#else
#include <avr/pgmspace.h>

constexpr unsigned char theEnd[] PROGMEM
{

};
#endif

void drawEnding()
{
	// Clear the screen
	arduboy.clear();

	#if FX_MODE == ON
		// Draw the ending graphic
		FX::drawBitmap(0, 0, theEnd, 0, dbmNormal);
	#else
		Sprites::drawOverwrite(0, 0, theEnd, 0);
	#endif

	// Update the display
	updateDisplay();
}

void awaitKeyEnding()
{ 
  arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
  updateDisplay(); 

  #if SOUND == ON
    sound.tone(600, 50);
    delay(50);

    sound.tone(1318, 50);
    delay(50);

    sound.tone(900, 50);
    delay(50);
  #endif

  bool blink { false };
  uint8_t counter { 0 };

  while(true)
  {
    if(arduboy.pressed(FIRE_BUTTON))
      break;

    if(arduboy.pressed(JUMP_BUTTON))
      break;

    #if SERIAL == ON
    if(Serial.available())
    {
      Serial.read();
      Serial.read();
      Serial.read();
      break;
    } 
    #endif

    ++counter;

    if(counter > 40)
    {
      counter = 0;
      blink = !blink;

      // Draw blinking cursor
      arduboy.fillRect(120, 55, 10, 10, blink ? WHITE : BLACK);
      updateDisplay();
    }

    delay(10);
  }

  arduboy.clear();
  updateDisplay();

  #if SERIAL == ON
  Serial.println("\n"); 
  #endif
}