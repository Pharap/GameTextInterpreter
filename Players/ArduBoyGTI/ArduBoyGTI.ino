// Arduboy GTI player

// Replace gti[] in Bytecode.h with your compiled game bytecode by using compiler.py and bin2array.py

#include "Arduboy.h"
#include "Intro.h"
#include "VirtualMachine.h"

void setup()
{
  arduboy.begin();
  arduboy.clear();

  showIntro();

  arduboy.clear();
  arduboy.display();

  #if USE_SERIAL == ON
  Serial.begin(9600);
  #endif
}

void loop()
{
  runVM();
}