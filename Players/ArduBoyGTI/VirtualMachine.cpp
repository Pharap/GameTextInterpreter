#include "VirtualMachine.h"

#include "Arduboy.h"
#include "Bytecode.h"
#include "CodeReading.h"
#include "Printing.h"
#include "Input.h"
#include "Ending.h"

char exita[80] {};
char exitb[80] {};

void runVM()
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
                updateDisplay();
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

        case Mode::Ending:
        {
          // Print text until a null character is found
          printText();

          // Wait for a key press
          awaitKey();

          // Draw ending bitmap
          drawEnding();

          // Wait for the final key press
          awaitKey(true);

          // Reset program counter
          pc = 0;

          // Exit the virtual machine
          return;
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
      updateDisplay();

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
      updateDisplay();

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
      updateDisplay();

      printCharacter('B');
      printCharacter(']');
      printCharacter(' ');
      updateDisplay();

      #if USE_SERIAL == ON
      Serial.print("\nB] ");
      #endif

      // Print the contents of the exitb buffer
      printString(exitb);

      // Draw a cursor box
      arduboy.fillRect(arduboy.getCursorX(), arduboy.getCursorY(), 8, 8, BLACK);
      updateDisplay();

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
      updateDisplay();
    }
  }
}