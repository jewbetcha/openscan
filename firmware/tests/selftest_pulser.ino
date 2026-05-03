// Bench test: fire the avalanche pulser at a low repetition rate.
//
// SAFETY: 905 nm is invisible. Before running:
//   1. Aim the laser into a beam dump or a non-reflective far wall.
//   2. Wear OD4+ 905 nm safety goggles.
//   3. No mirrors, glass, water, or polished metal in the beam path.
//   4. Confirm duty cycle stays well under 0.1%.
//
// This sketch fires once per second by default. Verify pulse shape on a
// fast scope (50 ohm into ~50 ns pulse width, clean rising edge).

constexpr uint8_t PIN_TRIG = 25;
constexpr uint16_t PULSE_US = 1;      // GPIO high time; pulser shapes the optical pulse
constexpr uint32_t PERIOD_MS = 1000;  // 1 Hz rep rate for safety during bring-up

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
  Serial.println("pulser selftest: confirm beam dump in place, goggles on");
  delay(3000);
}

void loop() {
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(PULSE_US);
  digitalWrite(PIN_TRIG, LOW);
  Serial.println("fire");
  delay(PERIOD_MS);
}
