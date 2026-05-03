# firmware

Arduino-ESP32 firmware for the openscan rangefinder.

Toolchain: **Arduino-ESP32** (Espressif's Arduino core for ESP32). Chosen over
ESP-IDF for v1 because the bench-test-first plan in `rangefinder_build.md`
rewards fast iteration, and SSD1306 / BLE / SPI libraries are well-supported.

See `../docs/flashing.md` for build and flash instructions.

## Layout

```
firmware/
  openscan/                    # Arduino sketch directory (matches .ino name)
    openscan.ino               # entry point: setup() + loop()
    config.h                   # pins, constants, calibration values
    pulser.h / pulser.cpp      # GPIO trigger for the avalanche pulser
    tdc.h    / tdc.cpp         # TDC7200 SPI driver
    sipm.h   / sipm.cpp        # bias enable / monitor (if MCU-controlled)
    display.h/ display.cpp     # SSD1306 OLED via I2C
    button.h / button.cpp      # debounced trigger button
    rangefinder.h / .cpp       # orchestrates fire->measure->filter->report
    ble.h    / ble.cpp         # BLE logging (Phase 4, stub for now)
  tests/
    selftest_tdc.ino           # fake START/STOP, verify TDC reads
    selftest_display.ino       # OLED hello-world
    selftest_button.ino        # button debounce check
    selftest_pulser.ino        # safe pulser fire (into beam dump)
```

Each module mirrors a Phase 1 subsystem so it can be bench-tested in isolation
before integration. Don't skip the `tests/` selftests just because integration
"feels close" — that's how parts get bricked.

## Build conventions

- C++ with Arduino conventions (`setup()`, `loop()`).
- One module per subsystem; modules expose a small `begin()` + a few action
  functions. No globals leaking across modules — pass state through the module.
- Pin assignments and tunables live in `config.h`. Never hardcode pins in
  module code.
- No `delay()` in the hot measurement path; use `micros()` / hardware timers.
- Comments only for non-obvious constraints (timing, hardware quirks).

## Dependencies

Installed via the Arduino Library Manager (see `docs/flashing.md`):

- **Adafruit SSD1306** + **Adafruit GFX** — OLED driver
- **NimBLE-Arduino** — lighter BLE stack than the built-in (Phase 4)

The TDC7200 driver is hand-rolled in `tdc.cpp` against the TI datasheet —
existing libraries are sparse and we want explicit control over Mode 1 vs
Mode 2 and calibration cycles.
