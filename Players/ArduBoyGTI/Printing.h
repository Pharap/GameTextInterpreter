#pragma once

#include "CodeReading.h"

void printCharacter(char character);

template<size_t size>
void printString(char (&buffer)[size])
{
  // Avoid a buffer overrun
  for(uint8_t index = 0; true; ++index)
  {
    // Read a character from the buffer
    const char character { buffer[index] };

    // Print the character
    printCharacter(character);

    // If the character is null
    if(character == '\0')
      // Exit
      return;
  }
}

// Print text until a null character is found
inline void printText()
{
  while(true)
  {
    const char character { readCharacter() };

    printCharacter(character);

    if(character == '\0')
      break;
  }
}