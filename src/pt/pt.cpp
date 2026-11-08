// I2C ADC
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#include "pt.h"

// Instantiate the ADCs. The number in their names are their I2C addresses.
// Change the names if you change their addresses
static Adafruit_ADS1015 ads_48;
static Adafruit_ADS1015 ads_49;

static PT pts[] = {
  // name,            *adc,    channel, pressureRange, voltageMin, voltageMax
  { "GN2",            &ads_48, 0,       5000.0,        0.5,        4.5 },
  { "LOX-UPSTREAM",   &ads_48, 1,       1000.0,        1.0,        5.0 },
  { "LNG-UPSTREAM",   &ads_48, 2,       1000.0,        1.0,        5.0 },
  { "LOX-DOWNSTREAM", &ads_48, 3,       1000.0,        1.0,        5.0 },
  { "LNG-DOWNSTREAM", &ads_49, 0,       1500.0,        0.5,        4.5 },
  { "LOX-DOME",       &ads_49, 1,       1500.0,        0.5,        4.5 },
  { "LNG-DOME",       &ads_49, 2,       1500.0,        0.5,        4.5 },
};

// All PTs divided by one PT
static const size_t NUM_PTS = sizeof(pts) / sizeof(pts[0]);

// quick sanity check
static inline bool valid(uint8_t id) {
  return id < NUM_PTS;
}

void setupPTs() {
  // Explicitly start the I2C bus on the exact schematic GPIO (NOT IC) pins for the sensor board (SDA = 21, SCL = 22)
  Wire.begin(21, 22);

  // Initialize the ADC hardware
  ads_48.begin(0x48);
  ads_49.begin(0x49);
}

uint8_t countPTs() {
  return NUM_PTS;
}

const PT* getPTInfo(uint8_t id) {
  // DANGER!!! really should never trigger this...
  if (!valid(id)) {
    Serial.println("ERROR: referencing unexisting PT, returning 0 as default.");
    return &pts[0];
  }
  return &pts[id];
}

// Note: 0.0 appears to indicate an error.
double readPT(uint8_t id) {
  if (!valid(id)) return 0.0;
  const PT& p = pts[id];

  // Let the Adafruit library handle the analogRange and voltageRange math
  int16_t adcData = p.adc->readADC_SingleEnded(p.channel);
  double voltage = p.adc->computeVolts(adcData);

  double denom = (p.voltageMax - p.voltageMin);
  if (denom <= 0.0) return 0.0; // still don't know why

  // Map the read voltage to the physical pressure range
  double pressure = ((voltage - p.voltageMin) / denom) * p.pressureRange;

  if (pressure <= 0.0) return 0.0;
  if (pressure > p.pressureRange) return p.pressureRange; // clamp
  return pressure;
}
