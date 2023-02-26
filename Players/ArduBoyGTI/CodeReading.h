#pragma once

#include "Settings.h"

#if FX_MODE == ON
#include "fxdata.h"
#else
#include "Bytecode.h"
#endif

extern uint16_t pc;

inline uint8_t readByte()
{
  #if FX_MODE == ON
  // Seek to the current pc position
  FX::seekData(gti + pc); 

  // Read the byte
  const uint8_t result { FX::readEnd() };
  #else
  // Read the byte
  const uint8_t result { pgm_read_byte(&gti[pc]) };
  #endif

  // Increment the program counter
  ++pc;

  // Return the result
  return result;
}

inline uint16_t readWord()
{
  #if FX_MODE == ON
  // Seek to the current pc position
  FX::seekData(gti + pc); 

  // Read the uint16_t
  const uint16_t result { FX::readPendingLastUInt16() };
  #else
  // Read the high byte
  const uint8_t high { readByte() };

  // Read the low byte
  const uint8_t low  { readByte() };

  // Combine the bytes into a single result
  const uint16_t result { (static_cast<uint16_t>(high) << 8) | (static_cast<uint16_t>(low) << 0) };
  #endif

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