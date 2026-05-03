#pragma once

// The SiPM bias is currently produced by the standalone XL6009 boost
// converter and is not switched by the MCU in v1. This module is a
// placeholder for a future MCU-controlled bias enable / monitor.

void sipm_begin();
void sipm_bias_enable(bool on);
bool sipm_bias_ok();
