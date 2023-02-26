#pragma once

#include <Arduboy2.h>
#include <ArduboyTones.h>

#include "Settings.h"

#if FX_MODE == ON
#include <ArduboyFX.h>
#endif

extern Arduboy2 arduboy;
extern ArduboyTones sound;

inline void updateDisplay()
{
	#if FX_MODE == ON
	FX::display()
	#else
	arduboy.display();
	#endif
}