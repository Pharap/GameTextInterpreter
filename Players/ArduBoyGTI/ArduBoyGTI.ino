// Arduboy GTI player

// replace char gti[]  with your compiled game bytecode by using compiler.py and bin2array.py -- it is currently silence.bin now

// Standard includes for Arduboy 
 
#include <Arduboy2.h>
#include <ArduboyTones.h>

#include <avr/pgmspace.h>

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

#include "Settings.h"
#include "Bytecode.h"

// Init system variables

uint8_t state = 0;
uint16_t room = 0;
uint8_t inst = 0;
uint8_t col = 0;
uint8_t row = 0;

extern const unsigned char arduino[];

PROGMEM const unsigned char arduino [] = {
0x3F, 0xFF, 0xFF, 0xFC, 0x40, 0x00, 0x00, 0x02, 0x89, 0x99, 0x54, 0x91, 0x95, 0x55, 0x56, 0xA9,
0x9D, 0x95, 0x55, 0xA9, 0x95, 0x59, 0xD4, 0x91, 0x40, 0x00, 0x00, 0x02, 0x3F, 0xFF, 0xFF, 0xFC
};

char exita[80] {};
char exitb[80] {};

uint16_t pc { 0 };

uint8_t readByte()
{
  // Read the byte
  const uint8_t result { pgm_read_byte(&gti[pc]) };

  // Increment the program counter
  ++pc;

  // Return the result
  return result;
}

uint16_t readWord()
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

char readCharacter()
{
  return readByte();
}

template<size_t size> void readString(char (&buffer)[size])
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

template<size_t size> void printString(char (&buffer)[size])
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
void printText()
{
  while(true)
  {
    const char character { readCharacter() };

    printCharacter(character);

    if(character == '\0')
      break;
  }
}

void setup() {
  arduboy.begin();
  arduboy.clear();
  showIntro();
  arduboy.clear();
  arduboy.display();
  #if USE_SERIAL == ON
  Serial.begin(9600);
  #endif
}

void loop() {
  startVM();
}

void showIntro()    // Show Arduino retro intro
{
   for(int i=-8; i<28; i=i+2) {
    arduboy.clear();
    arduboy.drawSlowXYBitmap(46,i, arduino, 32,8,1);
    arduboy.display();
    delay(1000/30);
  }  
  #if SOUND == ON
   sound.tone(987, 160);
    delay(160);
   sound.tone(1318, 400);
   delay(2000);
  #endif
}

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

void startVM()
{
  while(true)
  {
    // Read frame type
    const uint16_t type { readWord() };

    // Reset columns in text display
    col = 0;

    // Special frame
    if(type == 65535)
    {
      // Get the special packet mode
      const Mode mode { static_cast<Mode>(readByte()) };

      switch (mode)
      {
        case Mode::GameOver:
        {
          // Print text until a null character is found
          printText();
          
          // Print generic Game Over message
          arduboy.print(F("*GAME OVER*"));

          // If the serial port is turned on
          #if USE_SERIAL == ON
          // Print a message over serial
          Serial.print(F("*GAME OVER*"));
          #endif

          // Wait for a key press
          awaitKey();

          // Reset program counter
          pc = 0;

          // Exit the virtual machine
          return;
        }

        case Mode::Jump:
        {
          // Set the program counter to the address in 16 bit field
          pc = readWord();

          break;
        }

        case Mode::Effect:
        {
          // If the effects library is switched on
          #if SFXLIB == ON

          // Get the effect type
          const Effect effect { static_cast<Effect>(readByte()) };

          switch(effect)
          {
            // Low pitched rumbling sound
            case Effect::Rumble:
            {
              // If sound is turned on
              #if SOUND == ON
              // Do a low frequency sweep
              for(int x = 60; x < 150; x++)
              {
                sound.tone(x, 10);
                delay(10);
              }
              #endif

              break;
            }

            // Random lightning flash
            case Effect::Lightning:
            {
              // A small loop for lightning flashes
              for(int x = 0; x < 50; x++)
              {
                // A 1 in 10 chance of a flash
                arduboy.fillScreen((random(0, 10) == 0) ? WHITE : BLACK);
                arduboy.display();
                delay(20);
              }

              break;
            }
          }
          #endif
          break;
        }

        case Mode::Text:
        {
          // Print text until a null character is found
          printText();

          // Wait for a key press
          awaitKey();

          break;
        }
      }
    }
    // A room instead of a special frame
    else
    {
      // Get jump A address
      const uint16_t branchA { type };

      // Get jump B address
      const uint16_t branchB { readWord() };

      // Copy null-terminated string to exita buffer
      readString(exita);

      // Copy null-terminated string to exitb buffer
      readString(exitb);

      // Print the room description
      printText();

      #if SOUND == ON
      sound.tone(1318, 50); 
      #endif

      // Draw a cursor box
      arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
      arduboy.display();

      if(arduboy.getCursorY() < 48)
      {
        arduboy.print('\n'); 
        #if USE_SERIAL == ON
        Serial.println();
        #endif
      } 

      printCharacter('A');
      printCharacter(']');
      printCharacter(' ');
      arduboy.display();

      #if USE_SERIAL == ON
      Serial.print("\nA] ");
      #endif

      // Print the contents of the exita buffer
      printString(exita);

      #if SOUND == ON
      sound.tone(1318, 50); 
      #endif

      // Draw a cursor box
      arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
      arduboy.display();

      printCharacter('B');
      printCharacter(']');
      printCharacter(' ');
      arduboy.display();

      #if USE_SERIAL == ON
      Serial.print("\nB] ");
      #endif

      // Print the contents of the exitb buffer
      printString(exitb);

      // Draw a cursor box
      arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
      arduboy.display();

      // Wait for the user's input
      const int selection { awaitSelection() };

      switch(selection)
      {
        // If the user chose A
        case 0:
          // Take the first branch
          pc = branchA;
          break;

        // If the user chose B
        case 1:
          // Take the second branch
          pc = branchB;
          break;
      }

      arduboy.clear();
      arduboy.display();
    }
  }
}

constexpr uint8_t charsPerLine = 21;
// One extra to detect end of line word break
constexpr uint8_t linebufferSize = charsPerLine + 1;
constexpr uint8_t linebufferLastPos = charsPerLine; 

char lineBuffer[linebufferSize];

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
