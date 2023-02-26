#include "Printing.h"

#include "Arduboy.h"

constexpr uint8_t charsPerLine = 21;
// One extra to detect end of line word break
constexpr uint8_t linebufferSize = charsPerLine + 1;
constexpr uint8_t linebufferLastPos = charsPerLine; 

char lineBuffer[linebufferSize];

uint8_t col = 0;

void printCharacter(char character)
{
  // Copy character to buffer
  lineBuffer[col] = character;

  // If the character is an ordinary character
  // and the current position isn't the last...
  if ((character >= ' ') && (col < linebufferLastPos))
  {
    // Advance the buffer index
    ++col;

    // Exit the function
     return;
  }

  // If the cursor has gone past the bottom of the screen
  if (arduboy.getCursorY() >= HEIGHT)
  {
    constexpr size_t lastLineOffset { ((HEIGHT * WIDTH) / 8) - WIDTH };

    // Scroll the buffer up
    for (size_t index = 0; index < lastLineOffset; ++index)
      arduboy.sBuffer[index] =  arduboy.sBuffer[index + WIDTH];

    // Clear the last line
    for (uint8_t index = 0; index < WIDTH; ++index)
      arduboy.sBuffer[lastLineOffset + index] = 0;

    // Move the cursor up
    arduboy.setCursor(arduboy.getCursorX(), HEIGHT - 8);
  }

  uint8_t wordBreakIndex = col;

  // Try to find a suitable place to introduce a word break
  while (wordBreakIndex > 0)
  {
    if (lineBuffer[wordBreakIndex] <= ' ')
      break;

    --wordBreakIndex;
  }

  // If no word break was found
  if (wordBreakIndex == 0)
    wordBreakIndex = col;

  if (wordBreakIndex == linebufferLastPos)
  {
    // Exclude 1st character of next line
    --wordBreakIndex;

    // Remove space at start of next line
    if (lineBuffer[linebufferLastPos] == ' ')
      --col;
  }

  for (uint8_t index = 0; index <= wordBreakIndex; ++index)
  {
    character = lineBuffer[index];

    if(character >= ' ') 
    {
      arduboy.print(character);
      #if USE_SERIAL == ON
      Serial.print(character);
      #endif 
    }

    #if SOUND == ON
    sound.tone(1318, 2);
    #endif

    arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, WHITE); 
    arduboy.display();

    delay(50);
  }

  // Remaining characters for next line
  col -= wordBreakIndex;

  // Move remaining characters to the start of the buffer 
  for (uint8_t index = 0; index <= col; ++index)
    lineBuffer[index] = lineBuffer[wordBreakIndex + index + 1];

  arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
  arduboy.println();

  #if USE_SERIAL == ON
  Serial.print("\n"); 
  #endif
}