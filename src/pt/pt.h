#ifndef PT_H
#define PT_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

struct PT {
  const char*        name;
  Adafruit_ADS1015*  adc;
  uint8_t            channel;
  double             pressureRange;
  double             voltageMin;
  double             voltageMax;
};

// Initialize PT state
void setupPTs();

// Return total number of PTs
uint8_t countPTs();

// Get read-only PT descriptor for debugging
const PT* getPTInfo(uint8_t id);

// Read PSI from PT
double readPT(uint8_t id);

#endif
