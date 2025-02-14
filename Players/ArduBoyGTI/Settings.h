#pragma once

#define ON true
#define OFF false

// Enables or disables ArduboyFX mode:
// * ON - Read from the FX chip
// * OFF - Read from progmem
#define FX_MODE OFF

// 574 bytes (If support is disabled, bytecode is skipped properly)
#define SOUND ON

// 182 bytes (no impact on bytecode interpreting)
#define USE_SERIAL OFF

#define VARIABLES ON
#define MUSIC ON

// 496 bytes (If support is disabled, bytecode is skipped properly)
#define SFXLIB ON
#define VECTOR ON

// Button definition
#define FIRE_BUTTON B_BUTTON
#define JUMP_BUTTON A_BUTTON