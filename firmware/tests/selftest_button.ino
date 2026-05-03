// Bench test: verify the trigger button is wired with internal pull-up
// and produces clean debounced edges. Counter increments once per press.

constexpr uint8_t PIN_BUTTON = 27;
constexpr uint16_t DEBOUNCE_MS = 25;

bool last_stable = false;
bool last_raw = false;
uint32_t last_change = 0;
uint32_t presses = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void loop() {
  bool raw = (digitalRead(PIN_BUTTON) == LOW);
  uint32_t now = millis();
  if (raw != last_raw) {
    last_raw = raw;
    last_change = now;
  }
  if ((now - last_change) >= DEBOUNCE_MS && raw != last_stable) {
    last_stable = raw;
    if (raw) {
      presses++;
      Serial.printf("press %u\n", presses);
    }
  }
}
