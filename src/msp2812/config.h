#ifndef CONFIG_H
#define CONFIG_H

#define N_LEDS (30)
#define N_BYTES_PER_LED (3)


// You definitely want to uncomment and change this to your callsign.
#define CALLSIGN "N0CALL"

// The core voltage should be 0 for a coin cell, 2 or 3 for quality power.
#define COREVOLTAGE 3

// Uncomment this to emulate the SET button by holding + and - at once.
//#define EMULATESET

#endif // CONFIG_H
