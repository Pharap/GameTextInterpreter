// Arduboy GTI player

#define ON 1
#define OFF 0

#include <EEPROM.h>
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <ArduboyFX.h>

#include "fxdata/fxdata.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

// Game options
// Interpreter size tuning (turning off some interpreter support can save PROGMEM memory, but code must not contain related bytecodes (unless stated!)

#define SOUND ON             // 574 bytes (If support is disabled, bytecode is skipped properly)
#define USE_SERIAL OFF       // 182 bytes (no impact on bytecode interpreting)

#define VARIABLES ON
#define MUSIC ON
#define SFXLIB ON            // 496 bytes (If support is disabled, bytecode is skipped properly)
#define VECTOR ON

// Init system variables

uint8_t state = 0;
uint16_t room = 0;
uint8_t inst = 0;
uint8_t col = 0;
uint8_t row = 0;
uint8_t effect = 0;

uint8_t bt_a = 0;
uint8_t bt_b = 0;

unsigned char exita[80] = "";
unsigned char exitb[80] = "";

void setup() {
  arduboy.begin();
  FX::begin(FX_DATA_PAGE);    // initialise FX chip
  FX::display(CLEAR_BUFFER);
}

void loop() {
startVM(0);
}

void startVM(uint16_t pc) {
 while(1) {  
  FX::seekData(gti + pc);                                                                         // Search for selected byte
  bt_a = FX::readPendingUInt8();
  bt_b = FX::readEnd();
  //NEW
  uint16_t type = ((bt_a & 0xFF) << 8) | ((bt_b & 0xFF) << 0);                                    // Read in type of frame
  pc++;                                                                                           // bump program counter
  col = 0;                                                                                        // reset columns in text display
  if(type == 65535) {                                                                             // SPECIAL Frame Detected
     pc++;                                                                                        // bump program counter
     //NEW
     FX::seekData(gti + pc);                                                                      // Search for selected byte
     uint8_t mode = FX::readEnd();                                                                // get the special packet mode
     
     switch (mode) {                                                                              // switch based on what mode the frame is

       // 0x00 - Game Over

       case 0:                                                                                    // Game Over handler
           while(1) {                                                                             // loop over text until we hit a null
             pc++;                                                                                // bump program counter
             //NEW
             FX::seekData(gti + pc);                                                              // Search for selected byte
             uint8_t buff = FX::readEnd();                                                        // read a chracter
             printer(buff);                                                                       // print out the text using our text output handler
             if(buff == 0) { break; }                                                             // is it a null? then we are done printing text
           }
           arduboy.print("*GAME OVER*");                                                          // print generic Game Over message
           #if USE_SERIAL == ON
           Serial.print("*GAME OVER*");                                                           // if the serial port is turned on, print message over serial
           #endif
           anykey();                                                                              // wait for anykey
           col = 0;                                                                               // reset the columns (probably not needed)
           return;                                                                                // quit out of the VM
           break;                                                                                 // break that will not be executed
           
       // 0x05 - Jump    
           
       case 5:                                                                                    // Jump handler
           pc++;                                                                                  // bump the program counter
           //NEW
           FX::seekData(gti + pc);                                                                // Search for selected byte
           bt_a = FX::readPendingUInt8();
           bt_b = FX::readEnd();
           pc = ((bt_a & 0xFF) << 8) | ((bt_b & 0xFF) << 0);                                      // Set the program counter to the address in 16 bit field
           break;                                                                                 // switch break
           
       // 0x0D - Effects Library    

       case 13:                                                                                   // Effects Handler
       #if SFXLIB == ON                                                                           // If our effects library is switched on
            pc++;                                                                                 // Bump program counter
            //NEW
            FX::seekData(gti + pc);                                                               // Search for selected byte
            effect = FX::readEnd();                                                               // Get effect type
            
            switch(effect) {                                                                      // Effect switcher
            
               // 0x00 - Low pitched rumbling sound
            
               case 0:                                                                            // rumble handler
                   #if SOUND == ON                                                                // Is sound turned on?
                   for(int x = 60; x < 150; x++) {                                                // Low frequency sweep 
                   sound.tone(x, 10);  delay(10); }                                               // Play our low frequency sweep
                   #endif                                                                         // end sound check
                   break;                                                                         // switch break
                  
               // 0x01 - Random lightning flash   
                   
               case 1:                                                                            // Random lightning flash 
                   for(int x = 0; x < 50; x++) {                                                  // Small loop for our flashes
                     if(random(0,10) == 1) {                                                      // 1 in 10 chance of a flash
                            arduboy.fillScreen(WHITE);                                            // Fill screen with white
                            FX::display();                                                        // push it out
                     } 
                     else {                                                                       // Not a 1 in 10 chance?
                            arduboy.fillScreen(BLACK);                                            // Make screen black
                            FX::display();                                                        // Push it out
                     } 
                     delay(20);
                   }
                   break;                                                                         // switch break
            }  
            pc++;                                                                                 // Bump program counter
       #else                                                                                      // Otherwise, is the SFX mode disabled?
         pc++; pc++;                                                                              // Increment program counter so we don't lose position
       #endif
            break;                                                                                // switch break

       // 0x10 - Display page of text and wait for any key
       
       case 16:                                                                                   // text handler
           while(1) {                                                                             // print some text until we find a null
             pc++;                                                                                // Bump program counter
             //NEW
             FX::seekData(gti + pc);                                                              // Search for selected byte
             uint8_t buff = FX::readEnd();                                                        // Read a character
             printer(buff);                                                                       // Send text off to our printer 
             if(buff == 0) { break; }                                                             // Is it null? then break out of loop
           }         
           anykey();                                                                              // Wait for any key 
           pc++;                                                                                  // Bump program counter
           break;                                                                                 // Switch case break
    }
  }
  
  // We did not find a SPECIAL frame, so we assume it is a NORMAL frame (and per framing format, it must be)
  // We reuse the 'type' uint16_t variable as jump a's address
  
  else {    

     // Jump A description buffer
    
      pc++;                                                                                                   // bump program counter
      //NEW
      FX::seekData(gti + pc);                                                                                 // Search for selected byte
      bt_a = FX::readPendingUInt8();
      bt_b = FX::readEnd();
      uint16_t alterexit = ((bt_a & 0xFF) << 8) | ((bt_b & 0xFF) << 0);                                       // get jump b address
      pc++; pc++;                                                                                             // bump program counter
      int i = 0;                                                                                              // initalize i (to keep track of array position)
      while(1 == 1) {                                                                                         // fill jump a description buffer
        //NEW
        FX::seekData(gti + pc);                                                                               // Search for selected byte
        uint8_t buff = FX::readEnd();                                                                         // get a character
        if(buff == 0) { exita[i] = 0; break; }                                                                // Is it null? append the null to array then stop
        exita[i] = buff;                                                                                      // fill array with newest character
        i++; pc++;                                                                                            // bump array position and program counter
      }

     // Jump B description buffer
      
      pc++;                                                                                                   // bump program counter
      i = 0;                                                                                                  // reset array position
      while(1 == 1) {                                                                                         // fill jump b description buffer 
        //NEW                                                      
        FX::seekData(gti + pc);                                                                               // Search for selected byte
        uint8_t buff = FX::readEnd();                                                                         // get a character
        if(buff == 0) { exitb[i] = 0; break; }                                                                // Is it null? append the null to array then stop
        exitb[i] = buff;                                                                                      // fill array with newest character
        i++; pc++;                                                                                            // bump array position and program counter
      } 
      
      int x = 0;                                                                                              // define x (we could probably still reuse i?)
 
      // Main description printing (since the description goes before the jump selections)
      
      while(1) {                                                                                              // print the main description text, null terminated
             pc++;                                                                                            // bump program counter
             //NEW                                                 
             FX::seekData(gti + pc);                                                                          // Search for selected byte
             uint8_t buff = FX::readEnd();                                                                    // get character from main description
             printer(buff);                                                                                   // Print out our newest character
             if(buff == 0) { break; }                                                                         // did we find a null? stop printing description
      }  
      pc++;
      #if SOUND == ON
      sound.tone(1318, 50); 
      #endif
      arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,BLACK); FX::display();
      if(arduboy.getCursorY() < 48) { arduboy.print("\n"); 
      #if USE_SERIAL == ON
      Serial.println();
      #endif
      } 
        
      printer('A'); printer(']'); printer(' '); FX::display();
      #if USE_SERIAL == ON
      Serial.print("\nA] ");
      #endif
       x = 0;
       while(1) { 
             printer(exita[x]);
             if(exita[x] == 0) { break; } 
             x++; 
      }  
      x = 0; 
      #if SOUND == ON
      sound.tone(1318, 50); 
      #endif
      arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,BLACK); FX::display();
      printer('B'); printer(']'); printer(' '); FX::display();
      #if USE_SERIAL == ON
      Serial.print("\nB] ");
      #endif      
       while(1) { 
             printer(exitb[x]);
             if(exitb[x] == 0) { break; } 
             x++;
      } 
      arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,BLACK); FX::display();
      int selection = select();
      if(selection == 0) { pc = type; } 
      if(selection == 1) { pc = alterexit; } 
      FX::display(CLEAR_BUFFER);
      col = 0;
  }
}
  }

void settings() { anykey(); } 

constexpr uint8_t charsPerLine = 21;
constexpr uint8_t linebufferSize = charsPerLine + 1; // one extra to detect end of line word break
constexpr uint8_t linebufferLastPos = charsPerLine; 
static uint8_t lineBuffer[linebufferSize];

void printer(uint8_t character) {
    lineBuffer[col] = character;
    if ((character >= 32) & (col < (linebufferLastPos))) { //return no special characters and buffer not full
        ++col;
        return;  
    }
    if (arduboy.getCursorY() >= HEIGHT) {
        for (uint16_t i = 0; i < HEIGHT * WIDTH / 8 - WIDTH; i++) arduboy.sBuffer[i] =  arduboy.sBuffer[i+128]; //scroll up
        for (uint8_t i = 0; i < WIDTH; i++) arduboy.sBuffer[HEIGHT * WIDTH / 8 - WIDTH + i] = 0; //clear bottom line
        arduboy.setCursor(arduboy.getCursorX(),HEIGHT - 8);
    }
    uint8_t brk = col;
    while (brk> 0) {
        if (lineBuffer[brk] > 32) --brk;
        else break;
    }
    if (brk == 0) brk = col; //no word break
    if (brk == linebufferLastPos) {
        --brk; // exclude 1st character of next line
        if (lineBuffer[linebufferLastPos] == 32); --col; //remove space at start of next line
    }
    for (uint8_t i = 0; i <= brk; i++) {
        character = lineBuffer[i];
        if(character >= 32) 
        {
            arduboy.print((char)character);
            #if USE_SERIAL == ON
            Serial.print((char)character);
            #endif 
        }
        #if SOUND == ON
        sound.tone(1318, 2);
        #endif
        arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,WHITE); 
        FX::display();
        delay(50);
    }
    col -= brk; // remaining characters for next line
    for (uint8_t i = 0; i <= col; i++) lineBuffer[i] = lineBuffer[brk+i+1]; //move remaining character to start of buffer 
    arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,BLACK); arduboy.println();
    #if USE_SERIAL == ON
    Serial.print("\n"); 
    #endif
}

uint8_t select() { 
  uint8_t c = 0; uint8_t blink = 0; 
  #if SOUND == ON
     sound.tone(1318, 50);  delay(50);
    sound.tone(400, 50); delay(50);
    sound.tone(600, 50); delay(50); 
  #endif
  while(1) { 
   if(arduboy.pressed(B_BUTTON)) { return 1; } 
   if(arduboy.pressed(A_BUTTON)) { return 0; } 
   
       c++; 
    if(c > 40) { 
       if(blink == 1) {  
         arduboy.fillRect(120,55,10,10,WHITE); 
         FX::display(); 
         blink = 0; } 
       else { 
         arduboy.fillRect(120,55,10,10,BLACK); 
         FX::display();  
         blink = 1; 
       } 
     c = 0;
   }    
  delay(10); 
  }
}

void anykey() { 
  arduboy.fillRect(arduboy.getCursorX(),arduboy.getCursorY(),8,8,BLACK); FX::display(); 
  #if SOUND == ON
    sound.tone(600, 50);  delay(50);
    sound.tone(1318, 50); delay(50);
    sound.tone(900, 50); delay(50);
  #endif
    uint8_t c = 0;
    uint8_t blink = 0;  
  while(1) { 
   if(arduboy.pressed(B_BUTTON) || arduboy.pressed(A_BUTTON)) { break; } 
   #if SERIAL == ON
   if(Serial.available()) { Serial.read(); Serial.read(); Serial.read(); break; } 
   #endif
    c++; 
    if(c > 40) { 
       if(blink == 1) {  
         arduboy.fillRect(120,55,10,10,WHITE); 
         FX::display(); 
         blink = 0; } 
       else { 
         arduboy.fillRect(120,55,10,10,BLACK); 
         FX::display();  
         blink = 1; 
       } 
     c = 0;
   }
   delay(10);    
   }
  FX::display(CLEAR_BUFFER); 
  #if SERIAL == ON
  Serial.println("\n"); 
  #endif
}