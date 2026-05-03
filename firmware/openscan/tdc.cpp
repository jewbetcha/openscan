#include "tdc.h"
#include "config.h"

#include <SPI.h>

// TDC7200 register map (TI datasheet SNAS647).
namespace reg {
constexpr uint8_t CONFIG1            = 0x00;
constexpr uint8_t CONFIG2            = 0x01;
constexpr uint8_t INT_STATUS         = 0x02;
constexpr uint8_t INT_MASK           = 0x03;
constexpr uint8_t COARSE_CNTR_OVF_H  = 0x04;
constexpr uint8_t COARSE_CNTR_OVF_L  = 0x05;
constexpr uint8_t CLOCK_CNTR_OVF_H   = 0x06;
constexpr uint8_t CLOCK_CNTR_OVF_L   = 0x07;
constexpr uint8_t CLOCK_CNTR_STOP_MASK_H = 0x08;
constexpr uint8_t CLOCK_CNTR_STOP_MASK_L = 0x09;
constexpr uint8_t TIME1              = 0x10;
constexpr uint8_t CLOCK_COUNT1       = 0x11;
constexpr uint8_t TIME2              = 0x12;
constexpr uint8_t CALIBRATION1       = 0x1B;
constexpr uint8_t CALIBRATION2       = 0x1C;
}

namespace {

TdcMode current_mode = TdcMode::Mode1;
SPISettings spi_settings(cfg::SPI_HZ_TDC, MSBFIRST, SPI_MODE0);

void cs_low()  { digitalWrite(cfg::PIN_TDC_CS, LOW); }
void cs_high() { digitalWrite(cfg::PIN_TDC_CS, HIGH); }

void write8(uint8_t addr, uint8_t value) {
  SPI.beginTransaction(spi_settings);
  cs_low();
  SPI.transfer(addr | 0x40);  // auto-increment + write
  SPI.transfer(value);
  cs_high();
  SPI.endTransaction();
}

uint32_t read24(uint8_t addr) {
  SPI.beginTransaction(spi_settings);
  cs_low();
  SPI.transfer(addr & 0x3F);  // read, no auto-inc beyond 24 bits
  uint32_t v = 0;
  v |= (uint32_t)SPI.transfer(0) << 16;
  v |= (uint32_t)SPI.transfer(0) <<  8;
  v |= (uint32_t)SPI.transfer(0);
  cs_high();
  SPI.endTransaction();
  return v;
}

uint8_t read8(uint8_t addr) {
  SPI.beginTransaction(spi_settings);
  cs_low();
  SPI.transfer(addr & 0x3F);
  uint8_t v = SPI.transfer(0);
  cs_high();
  SPI.endTransaction();
  return v;
}

}  // namespace

void tdc_begin() {
  pinMode(cfg::PIN_TDC_CS, OUTPUT);
  pinMode(cfg::PIN_TDC_INT, INPUT);
  pinMode(cfg::PIN_TDC_ENABLE, OUTPUT);
  cs_high();

  digitalWrite(cfg::PIN_TDC_ENABLE, LOW);
  delay(5);
  digitalWrite(cfg::PIN_TDC_ENABLE, HIGH);
  delay(10);

  SPI.begin(cfg::PIN_TDC_SCK, cfg::PIN_TDC_MISO, cfg::PIN_TDC_MOSI, cfg::PIN_TDC_CS);

  // TODO: configure CONFIG2 (calibration periods, num stops, averaging).
  write8(reg::CONFIG2, 0x40);  // CALIBRATION2_PERIODS=10, NUM_STOP=1
  write8(reg::INT_MASK, 0x01); // new measurement interrupt only
}

void tdc_set_mode(TdcMode mode) {
  current_mode = mode;
}

void tdc_arm() {
  uint8_t cfg1 = 0x00;
  cfg1 |= (current_mode == TdcMode::Mode2) ? 0x02 : 0x00;  // MEAS_MODE
  cfg1 |= 0x80;  // START_MEAS
  write8(reg::INT_STATUS, 0x1F);  // clear interrupts
  write8(reg::CONFIG1, cfg1);
}

bool tdc_wait_for_done(uint32_t timeout_us) {
  uint32_t start = micros();
  while ((micros() - start) < timeout_us) {
    if (digitalRead(cfg::PIN_TDC_INT) == LOW) return true;
  }
  return false;
}

TdcResult tdc_read() {
  TdcResult r{};
  r.ticks       = read24(reg::TIME1);
  r.calibration = read24(reg::CALIBRATION1);
  r.ok          = (r.ticks != 0);
  return r;
}

float tdc_ticks_to_seconds(const TdcResult& r) {
  // TODO: implement full Mode 1/2 conversion using CALIBRATION1/2 + CLOCK_COUNT1
  // per TDC7200 datasheet equations. This stub is a placeholder for bench bring-up.
  if (r.calibration == 0) return 0.0f;
  return 0.0f;
}
