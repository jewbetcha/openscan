#include "button.h"
#include "config.h"

namespace {
bool last_stable = false;
bool last_raw = false;
uint32_t last_change_ms = 0;
bool edge_pending = false;
}

void button_begin() {
  pinMode(cfg::PIN_BUTTON, INPUT_PULLUP);
  last_raw = (digitalRead(cfg::PIN_BUTTON) == LOW);
  last_stable = last_raw;
  last_change_ms = millis();
}

static void update() {
  bool raw = (digitalRead(cfg::PIN_BUTTON) == LOW);
  uint32_t now = millis();
  if (raw != last_raw) {
    last_raw = raw;
    last_change_ms = now;
  }
  if ((now - last_change_ms) >= cfg::BUTTON_DEBOUNCE_MS && raw != last_stable) {
    last_stable = raw;
    if (raw) edge_pending = true;
  }
}

bool button_pressed_edge() {
  update();
  if (edge_pending) {
    edge_pending = false;
    return true;
  }
  return false;
}

bool button_held() {
  update();
  return last_stable;
}
