#pragma once

#include "Bytecode.h"

extern uint16_t pc;

inline uint8_t readByte()
{
  // Read the byte
  const uint8_t result { pgm_read_byte(&gti[pc]) };

  // Increment the program counter
  ++pc;

  // Return the result
  return result;
}

inline uint16_t readWord()
{
  // Read the high byte
  const uint8_t high { readByte() };

  // Read the low byte
  const uint8_t low  { readByte() };

  // Combine the bytes into a single result
  const uint16_t result { (static_cast<uint16_t>(high) << 8) | (static_cast<uint16_t>(low) << 0) };

  // Return the result
  return result;
}

inline char readCharacter()
{
  return readByte();
}

template<size_t size>
void readString(char (&buffer)[size])
{
  // Avoid a buffer overrun
  for(uint8_t index = 0; index < size; ++index)
  {
    // Read a character from the input
    const char character { readCharacter() };

    // Copy the character to the buffer
    buffer[index] = character;

    // If the character is null
    if(character == '\0')
      // Exit
      return;
  }
}