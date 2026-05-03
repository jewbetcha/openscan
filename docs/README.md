# docs

Build log, datasheets-of-record, calibration notes, and how-tos.

## Index

- [`flashing.md`](flashing.md) — install the toolchain and flash firmware to the ESP32.

## Conventions

- Add new entries chronologically when relevant (e.g. `2026-05-02_first-pulse-scope.md`).
- Drop oscilloscope captures and photos under `docs/captures/` with the date in the filename.
- Calibration constants live in `firmware/openscan/config.h` — record the source measurement here and reference it in the commit that updates the constant.
