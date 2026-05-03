#include "rangefinder.h"
#include "config.h"
#include "pulser.h"
#include "tdc.h"

#include <algorithm>

void rangefinder_begin() {
  tdc_set_mode(TdcMode::Mode2);
}

RangeResult rangefinder_measure() {
  RangeResult out{};
  out.shots = cfg::SHOTS_PER_MEASUREMENT;

  uint32_t samples[cfg::SHOTS_PER_MEASUREMENT];
  uint8_t n = 0;

  for (uint8_t i = 0; i < cfg::SHOTS_PER_MEASUREMENT; ++i) {
    tdc_arm();
    pulser_fire();
    if (tdc_wait_for_done(cfg::MEASUREMENT_TIMEOUT_MS * 1000UL)) {
      TdcResult r = tdc_read();
      if (r.ok) samples[n++] = r.ticks;
    }
    delayMicroseconds(cfg::INTER_SHOT_US);
  }

  out.hits = n;
  if (n < (cfg::SHOTS_PER_MEASUREMENT / 4)) return out;

  std::sort(samples, samples + n);
  uint32_t median = samples[n / 2];
  out.median_ticks = median;

  // TODO: convert median ticks -> seconds using tdc_ticks_to_seconds + cal offset.
  // Placeholder linear scaling; replace once datasheet conversion is implemented.
  float t_seconds = (float)(int32_t)(median - cfg::TDC_CALIBRATION_OFFSET_TICKS) * 55e-12f;
  out.distance_m = (t_seconds * cfg::SPEED_OF_LIGHT_M_PER_S) * 0.5f;
  out.ok = (out.distance_m > 0.0f);
  return out;
}
