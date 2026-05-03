#pragma once

#include <Arduino.h>

void display_begin();
void display_show_message(const char* msg);
void display_show_distance_yards(float yards);
void display_show_raw_ticks(uint32_t ticks);
