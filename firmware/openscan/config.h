#pragma once

#include <Arduino.h>

namespace cfg {

constexpr uint8_t PIN_PULSER_TRIG = 25;
constexpr uint8_t PIN_BUTTON = 27;

constexpr uint8_t PIN_TDC_CS = 5;
constexpr uint8_t PIN_TDC_INT = 26;
constexpr uint8_t PIN_TDC_ENABLE = 33;
constexpr uint8_t PIN_TDC_SCK = 18;
constexpr uint8_t PIN_TDC_MISO = 19;
constexpr uint8_t PIN_TDC_MOSI = 23;

constexpr uint8_t PIN_OLED_SDA = 21;
constexpr uint8_t PIN_OLED_SCL = 22;
constexpr uint8_t OLED_I2C_ADDR = 0x3C;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;

constexpr uint32_t SPI_HZ_TDC = 8'000'000;

constexpr uint8_t SHOTS_PER_MEASUREMENT = 16;
constexpr uint16_t INTER_SHOT_US = 200;
constexpr uint16_t PULSE_WIDTH_US = 1;
constexpr uint32_t MEASUREMENT_TIMEOUT_MS = 50;

constexpr float SPEED_OF_LIGHT_M_PER_S = 299'792'458.0f;

constexpr int32_t TDC_CALIBRATION_OFFSET_TICKS = 0;

constexpr uint16_t BUTTON_DEBOUNCE_MS = 25;

}  // namespace cfg
