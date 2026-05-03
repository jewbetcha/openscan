#pragma once

#include <Arduino.h>

// BLE logging is a Phase 4 feature. Kept as a stub so callers can compile
// against a stable interface today.

void ble_begin();
void ble_log_shot(uint32_t ticks_raw, float distance_m);
void ble_log_message(const char* msg);
