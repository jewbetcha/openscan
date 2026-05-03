#pragma once

#include <Arduino.h>

struct RangeResult {
  bool ok;
  float distance_m;
  uint8_t hits;   // shots that produced a STOP
  uint8_t shots;  // total shots fired
  uint32_t median_ticks;
};

void rangefinder_begin();
RangeResult rangefinder_measure();
