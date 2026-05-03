# openscan

A DIY 905 nm pulsed time-of-flight laser rangefinder targeting golf-distance
ranging (200+ yards) on a non-cooperative target like a flagstick. Companion
project to [Openflight](https://github.com/jewbetcha/openflight), the DIY
launch monitor.

> **Status: work in progress.** Parts are still on order. Nothing here is
> proven to work yet — schematics, firmware, and procedures are best-effort
> and will change as the build progresses. Do not treat any of this as a
> reference design until the build log says otherwise.

[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-support%20the%20build-ffdd00?style=for-the-badge&logo=buymeacoffee&logoColor=black)](https://buymeacoffee.com/colemangolfs)

If you want to help fund parts (lasers, SiPMs, lenses, the inevitable
"oops that's a $100 part" moments), the link above is the best way.

## What it does

Fires a sub-100 ns 905 nm laser pulse, times the round trip of the returning
photons with a TDC7200 (~55 ps resolution), and reports the distance on a
small OLED. ~1 yard accuracy needs ~3 ns timing resolution, which the TDC
provides with margin to spare.

## Architecture

```text
[ESP32] --TRIG--> [Avalanche pulser] --> [905nm laser] --> [Collimator] --> Target
   |                                                                          |
   |                                                                          v
   |                                                                 [Receive lens]
   |                                                                          |
   |                                                                          v
   |       [TDC7200] <-- STOP -- [Comparator] <-- [RF amp] <-- [SiPM + bias supply]
   |           |
   +-- SPI ----+
   +-- I2C --> [SSD1306 OLED]
   +-- GPIO -> [Trigger button]
```

Full hardware plan, BOM (~$290), and safety notes live in
[`rangefinder_build.md`](rangefinder_build.md).

## Repo layout

- [`rangefinder_build.md`](rangefinder_build.md) — hardware build plan, BOM,
  safety, known risks. Source of truth for the hardware.
- [`firmware/`](firmware/) — Arduino-ESP32 firmware, one module per subsystem,
  plus per-subsystem bench self-tests under `firmware/tests/`.
- [`docs/`](docs/) — build log, scope captures, calibration notes. Start with
  [`docs/flashing.md`](docs/flashing.md) to flash the ESP32.
- [`CLAUDE.md`](CLAUDE.md) — guidance for AI coding assistants working in
  this repo.

## Safety

905 nm is invisible — there is no blink reflex. Read the safety section of
`rangefinder_build.md` before powering anything on. Use OD4+ 905 nm goggles
during bench testing, aim into a beam dump, never point at people, mirrors,
glass, water, or polished metal. The SiPM dies instantly if biased and
mishandled.

## License

TBD. Treat the contents of this repo as "look but don't redistribute" until
a license file lands.

## Support

If this project is useful to you or you just want to follow along:

[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-ffdd00?style=for-the-badge&logo=buymeacoffee&logoColor=black)](https://buymeacoffee.com/colemangolfs)
