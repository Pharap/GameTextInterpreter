#include "Input.h"

#include "Arduboy.h"
#include "Settings.h"

uint8_t awaitSelection()
{
  #if SOUND == ON
    sound.tone(1318, 50);
    delay(50);

    sound.tone(400, 50);
    delay(50);

    sound.tone(600, 50);
    delay(50); 
  #endif

  bool blink { false };
  uint8_t counter { 0 };

  while(true)
  {
    if(arduboy.pressed(FIRE_BUTTON))
      return 1;

    if(arduboy.pressed(JUMP_BUTTON))
      return 0;

    ++counter;

    if(counter > 40)
    {
      counter = 0;
      blink = !blink;

      // Draw blinking cursor
      arduboy.fillRect(120, 55, 10, 10, blink ? WHITE : BLACK);
      arduboy.display();
    }

    delay(10);
  }
}

void awaitKey()
{ 
  arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
  arduboy.display(); 

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
      arduboy.display();
    }

    delay(10);
  }

  arduboy.clear();
  arduboy.display();

  #if SERIAL == ON
  Serial.println("\n"); 
  #endif
}