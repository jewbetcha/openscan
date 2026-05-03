#pragma once

#include <Arduino.h>

enum class TdcMode : uint8_t {
  Mode1 = 1,  // <500 ns expected ToF, short range
  Mode2 = 2,  // longer ToF, uses ring oscillator counter
};

struct TdcResult {
  bool     ok;
  uint32_t ticks;       // raw TIME1 register
  uint32_t calibration; // raw CALIBRATION1/2 for ps conversion
};

void   tdc_begin();
void   tdc_set_mode(TdcMode mode);
void   tdc_arm();                    // configures + waits-for-trigger
bool   tdc_wait_for_done(uint32_t timeout_us);
TdcResult tdc_read();
float  tdc_ticks_to_seconds(const TdcResult& r);
