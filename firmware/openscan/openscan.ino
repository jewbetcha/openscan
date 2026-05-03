#include "config.h"
#include "button.h"
#include "display.h"
#include "pulser.h"
#include "tdc.h"
#include "rangefinder.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}
  Serial.println(F("openscan boot"));

  display_begin();
  button_begin();
  pulser_begin();
  tdc_begin();
  rangefinder_begin();

  display_show_message("ready");
}

void loop() {
  if (button_pressed_edge()) {
    display_show_message("ranging...");
    RangeResult r = rangefinder_measure();
    if (r.ok) {
      display_show_distance_yards(r.distance_m * 1.09361f);
    } else {
      display_show_message("no return");
    }
  }
}
