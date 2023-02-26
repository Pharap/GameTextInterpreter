#pragma once

#include <stdint.h>

enum class Mode : uint8_t
{
  GameOver = 0x00,
  Jump = 0x05,
  Effect = 0x0D,
  Text = 0x10,
};

enum class Effect : uint8_t
{
  Rumble = 0x00,
  Lightning = 0x01,
};

void runVM();