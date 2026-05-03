// Bench test: bring up the TDC7200 over SPI.
// Drives a software-generated START and STOP via two GPIOs jumpered into
// the TDC START/STOP pins to verify register I/O and a non-zero TIME1.
// Expect: serial output of "tdc ticks: <N>" with N > 0 and stable.

#include <SPI.h>

constexpr uint8_t PIN_TDC_CS = 5;
constexpr uint8_t PIN_TDC_INT = 26;
constexpr uint8_t PIN_TDC_ENABLE = 33;
constexpr uint8_t PIN_FAKE_START = 32;
constexpr uint8_t PIN_FAKE_STOP = 4;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TDC_CS, OUTPUT);
  digitalWrite(PIN_TDC_CS, HIGH);
  pinMode(PIN_TDC_INT, INPUT);
  pinMode(PIN_TDC_ENABLE, OUTPUT);
  pinMode(PIN_FAKE_START, OUTPUT);
  pinMode(PIN_FAKE_STOP, OUTPUT);
  digitalWrite(PIN_FAKE_START, LOW);
  digitalWrite(PIN_FAKE_STOP, LOW);

  digitalWrite(PIN_TDC_ENABLE, LOW);
  delay(5);
  digitalWrite(PIN_TDC_ENABLE, HIGH);
  delay(10);

  SPI.begin(18, 19, 23, PIN_TDC_CS);

  // CONFIG2: 10 calibration periods, single stop
  SPI.beginTransaction(SPISettings(8'000'000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_TDC_CS, LOW);
  SPI.transfer(0x01 | 0x40);
  SPI.transfer(0x40);
  digitalWrite(PIN_TDC_CS, HIGH);
  SPI.endTransaction();

  Serial.println("tdc selftest ready");
}

uint32_t read24(uint8_t addr) {
  SPI.beginTransaction(SPISettings(8'000'000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_TDC_CS, LOW);
  SPI.transfer(addr & 0x3F);
  uint32_t v = 0;
  v |= (uint32_t)SPI.transfer(0) << 16;
  v |= (uint32_t)SPI.transfer(0) << 8;
  v |= (uint32_t)SPI.transfer(0);
  digitalWrite(PIN_TDC_CS, HIGH);
  SPI.endTransaction();
  return v;
}

void write8(uint8_t addr, uint8_t v) {
  SPI.beginTransaction(SPISettings(8'000'000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_TDC_CS, LOW);
  SPI.transfer(addr | 0x40);
  SPI.transfer(v);
  digitalWrite(PIN_TDC_CS, HIGH);
  SPI.endTransaction();
}

void loop() {
  write8(0x02, 0x1F);  // clear interrupts
  write8(0x00, 0x82);  // CONFIG1: Mode 2 + START_MEAS

  digitalWrite(PIN_FAKE_START, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_FAKE_START, LOW);
  delayMicroseconds(50);
  digitalWrite(PIN_FAKE_STOP, HIGH);
  delayMicroseconds(2);
  digitalWrite(PIN_FAKE_STOP, LOW);

  uint32_t start = micros();
  while (digitalRead(PIN_TDC_INT) == HIGH && (micros() - start) < 5000) {
  }

  uint32_t ticks = read24(0x10);
  Serial.printf("tdc ticks: %u\n", ticks);
  delay(500);
}
