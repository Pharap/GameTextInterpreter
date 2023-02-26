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