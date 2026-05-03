# CLAUDE.md

Guidance for Claude / agentic assistants working in this repository.

## Project: openscan

A DIY 905nm pulsed time-of-flight laser rangefinder targeting golf-distance ranging
(200+ yards) on a non-cooperative target like a flagstick. Companion project to
[Openflight](https://github.com/jewbetcha/openflight), the DIY launch monitor.

The full hardware build plan, BOM, safety notes, and open questions live in
`rangefinder_build.md`. That document is the source of truth for the hardware
design — read it before doing anything substantive in this repo.

## Goals

1. Build a working hand-held rangefinder that hits ~1 yard accuracy out to 200+ yards
   on a flagstick.
2. Keep the design DIY-reproducible: off-the-shelf parts, breadboard-friendly
   bring-up, total BOM ~$290.
3. Stay eye-safe (Class 1 operation under IEC 60825-1) at every stage of testing.
4. Generate a documented build log — oscilloscope captures, alignment notes, range
   data — usable as content alongside the OpenLaunch project.
5. Eventually log measurements over BLE for analysis and comparison against
   commercial units (Bushnell, Garmin).

## System architecture

```text
[ESP32] --TRIG--> [Avalanche pulser] --> [905nm laser diode] --> [Collimator] --> Target
   |                                                                                |
   |                                                                                v
   |                                                                      [Receive lens]
   |                                                                                |
   |                                                                                v
   |          [TDC7200] <-- STOP -- [Comparator] <-- [RF amp] <-- [SiPM + bias supply]
   |              |
   +-- SPI -------+
   +-- I2C --> [SSD1306 OLED]
   +-- GPIO -> [Trigger button]
```

Operating principle: ESP32 fires a sub-100 ns laser pulse and simultaneously starts
the TDC7200. Returning photons hit the SiPM, are amplified and discriminated by a
comparator, and stop the TDC. Distance = (time x c) / 2. ~1 yard accuracy needs
~3 ns timing resolution; the TDC7200 gives ~55 ps.

## Key subsystems

- **Laser transmitter** — SPL PL90AT03 905 nm diode driven by a 2N3904 avalanche
  pulser, ~50 ns pulse, 10–15 A peak, < 0.1% duty cycle.
- **Optics** — CAY046 aspheric collimator on transmit, 50 mm convex lens on receive
  (PATIKIL initially, possibly Thorlabs LA1131 later). Target beam divergence
  < 5 mrad. Boresight TX/RX to ~1 deg.
- **Detector** — MicroFC-SMTPA-60035 SiPM at ~27 V bias from an XL6009 boost
  converter with RC filter (1 kΩ + 10 µF tant + 100 nF cer).
- **Analog chain** — SiPM fast output AC-coupled (100 pF) into GALI-5+ MMIC amp,
  then into TLV3501 comparator with adjustable threshold.
- **Timing** — MIKROE-4770 TDC7200 click over SPI. Mode 1 vs Mode 2 selection
  matters for short vs long range.
- **MCU + UI** — ESP32 DevKit, SSD1306 0.96" I2C OLED, tactile button on GPIO with
  internal pull-up.
- **Power** — 18650 cell, TP4056 USB-C charger w/ DW01 protection, XL6009 boost for
  SiPM bias.

## Build phases (current state: pre-parts-arrival)

1. **Phase 1 — subsystem bench testing.** Validate bias supply, laser pulser, SiPM
   chain, and ESP32 + TDC + display *independently*, in that order. Do not skip
   bench testing in favor of integration.
2. **Phase 2 — optical alignment.** Collimator + receive lens + boresight.
3. **Phase 3 — system integration.** Calibrate timing offset against a known
   distance, characterize linearity at 10/50/100 m, tune comparator threshold.
4. **Phase 4 — field testing.** Flagstick at the driving range, BLE logging,
   compare against a commercial unit.

Pre-parts work the agent can do today:

- Read OSRAM SPL PL90_3 Range Finder app note for the avalanche pulser topology.
- Read Onsemi MicroFC datasheet (fast-output pin, bias / overvoltage specs).
- Read TI TDC7200 datasheet (Mode 1 vs Mode 2, edge timing).
- Sketch the 2N3904 avalanche pulser schematic with component values and check
  against the OSRAM app note.

## Repo layout

```text
openscan/
  README.md                  # placeholder
  rangefinder_build.md       # hardware build plan, BOM, safety, risks
  CLAUDE.md                  # this file
  firmware/
    README.md                # firmware overview + conventions
    openscan/                # main Arduino sketch
      openscan.ino
      config.h               # all pins + tunables live here
      pulser.{h,cpp}         # GPIO trigger for the avalanche pulser
      tdc.{h,cpp}            # TDC7200 SPI driver (in-tree)
      sipm.{h,cpp}           # placeholder for future MCU bias control
      display.{h,cpp}        # SSD1306 OLED via I2C
      button.{h,cpp}         # debounced trigger button
      rangefinder.{h,cpp}    # fire -> measure -> filter -> report
      ble.{h,cpp}            # Phase 4 BLE logging stub
    tests/                   # one .ino per Phase 1 subsystem self-test
      selftest_display.ino
      selftest_button.ino
      selftest_tdc.ino
      selftest_pulser.ino    # SAFETY notes inside
  docs/
    README.md
    flashing.md              # IDE + arduino-cli flashing instructions
```

Not yet present (introduce only when needed, after confirming with the user):

- `hardware/` — schematic and PCB sources (KiCad preferred).
- `tools/` — host-side scripts (calibration, BLE log analysis, etc.).

## Toolchain choice

Firmware uses **Arduino-ESP32** (Espressif's Arduino core), not ESP-IDF. The
bench-test-first plan rewards fast iteration, and SSD1306 / BLE / SPI libraries
are mature on this stack. Revisit if we hit timing limits the Arduino layer
can't solve (e.g. need RMT-driven hardware-precise pulse generation).

Dependencies (installed via Library Manager, see `docs/flashing.md`):

- Adafruit SSD1306 + Adafruit GFX
- NimBLE-Arduino (Phase 4 only)

The TDC7200 driver is hand-rolled in `firmware/openscan/tdc.cpp` against the TI
datasheet — keep it that way; we want explicit control over Mode 1 vs Mode 2,
calibration cycles, and edge timing.

## CI

Two GitHub Actions workflows run on push to `main` and on every PR:

- **`.github/workflows/firmware.yml`**
  - `clang-format` — dry-run check against `.clang-format` (Google base, 2-space, 100 col).
  - `cppcheck` — warning/style/performance/portability, `--error-exitcode=1`, std=c++17, scoped to `firmware/openscan`.
  - `compile` (matrix) — `arduino-cli compile --fqbn esp32:esp32:esp32` for the main sketch and every selftest. The job stages each `firmware/tests/*.ino` into a same-named subfolder before compiling because Arduino requires that layout.
- **`.github/workflows/docs.yml`**
  - `markdownlint` against `.markdownlint.json` (line-length and HTML rules disabled to keep the build doc readable).
  - `markdown-link-check` against every `*.md`, configured in `.github/markdown-link-check.json` (treats 403/429 as alive to avoid flakes from datasheet hosts).

When adding a new selftest sketch, drop it under `firmware/tests/` as `selftest_<name>.ino` and add the matching matrix entry in `firmware/.github/workflows/firmware.yml`. When adding a new firmware module, no workflow change is needed — `cppcheck` and the main-sketch compile pick it up automatically.

## Working conventions

- Treat `rangefinder_build.md` as authoritative for hardware decisions. If a change
  is needed, edit that file rather than letting CLAUDE.md drift.
- Keep BOM changes in `rangefinder_build.md` and update totals.
- When writing firmware, prefer small, testable modules per subsystem
  (`pulser`, `tdc`, `display`, `button`, `ble`) so they can be unit-bench-tested
  matching the Phase 1 plan.
- Log oscilloscope captures and measurement data into `docs/` with dates so the
  build log stays chronological.
- No comments unless they capture a non-obvious constraint, invariant, or
  workaround. Identifiers should carry the meaning.

## Safety — must-follow rules

These come from `rangefinder_build.md`. They are not optional and must be
respected in any code, doc, or procedural change suggested.

### Eye safety (905 nm is invisible — no blink reflex)

- Treat the laser as live whenever powered.
- Run the IEC 60825-1 eye-safety math (peak power x pulse width x rep rate, plus
  pulsed MPE) before pointing the device at any human-accessible zone.
- Never point at people, mirrors, glass, water, or polished metal.
- OD4+ 905 nm safety goggles during bench testing.
- Aim into a beam dump or far wall during pulser testing.

### SiPM survival

- Bias OFF whenever soldering, rewiring, or probing near the SiPM.
- Bring bias up slowly and verify with a meter before connecting the SiPM.
- One overvoltage event = dead $100 part.

### Battery safety

- Charge 18650s on a non-flammable surface; do not leave unattended initially.
- If a cell warms during charge or discharge, stop immediately.
- TEKOWEE cells are unverified; prefer Samsung 30Q or Molicel P26A long-term.

## Known risks / open questions

Tracked in `rangefinder_build.md`; summarized:

1. CAY046 collimator may not give tight enough divergence for 200+ yards.
2. PATIKIL receive lens is uncoated; likely first optical upgrade.
3. SiPM dark counts may force coincidence detection in firmware.
4. Direct sun will saturate the receiver — likely needs a 905 nm bandpass filter.
5. 2N3904 avalanche pulser jitter adds range error; GaN FET driver is the v2 path.

When firmware design touches any of these, call the trade-off out explicitly.

## What "done" looks like for early milestones

- M1: Bias supply holds 28.0 V steady under no load with RC filter installed.
- M2: Scope capture of clean ~50 ns laser pulse into a beam dump.
- M3: Comparator produces clean digital pulses from a slow LED test source.
- M4: ESP32 + TDC + OLED running a fake-measurement self-test on the bench.
- M5: First end-to-end measurement on a wall at 10 m, with a calibrated offset.
- M6: 50 yd flagstick measurement matching a commercial unit within 1 yd.

## For the agent

- Default to small, reversible changes; mirror the bench-test-first hardware
  approach in any code/doc work.
- Before adding dependencies, toolchains, or new directories, confirm with the
  user.
- When in doubt about a hardware spec, defer to `rangefinder_build.md` and the
  linked datasheets, not to general knowledge.
