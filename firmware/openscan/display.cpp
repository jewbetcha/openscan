#include "display.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace {
Adafruit_SSD1306 oled(cfg::OLED_WIDTH, cfg::OLED_HEIGHT, &Wire, -1);
bool ready = false;
}

void display_begin() {
  Wire.begin(cfg::PIN_OLED_SDA, cfg::PIN_OLED_SCL);
  ready = oled.begin(SSD1306_SWITCHCAPVCC, cfg::OLED_I2C_ADDR);
  if (!ready) {
    Serial.println(F("oled init failed"));
    return;
  }
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.println(F("openscan"));
  oled.display();
}

void display_show_message(const char* msg) {
  if (!ready) return;
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(0, 16);
  oled.println(msg);
  oled.display();
}

void display_show_distance_yards(float yards) {
  if (!ready) return;
  oled.clearDisplay();
  oled.setTextSize(3);
  oled.setCursor(0, 8);
  oled.print(yards, 1);
  oled.setTextSize(2);
  oled.setCursor(0, 44);
  oled.print(F("yd"));
  oled.display();
}

void display_show_raw_ticks(uint32_t ticks) {
  if (!ready) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print(F("ticks: "));
  oled.println(ticks);
  oled.display();
}
