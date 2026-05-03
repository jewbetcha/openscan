#include "pulser.h"
#include "config.h"

void pulser_begin() {
  pinMode(cfg::PIN_PULSER_TRIG, OUTPUT);
  digitalWrite(cfg::PIN_PULSER_TRIG, LOW);
}

void pulser_fire() {
  digitalWrite(cfg::PIN_PULSER_TRIG, HIGH);
  delayMicroseconds(cfg::PULSE_WIDTH_US);
  digitalWrite(cfg::PIN_PULSER_TRIG, LOW);
}
