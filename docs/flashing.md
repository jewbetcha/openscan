# Flashing the openscan firmware

Target board: **ESP32 DevKit** (Lonely Binary 3-pack), ESP32-WROOM-32 module.
Toolchain: **Arduino-ESP32** core, via either the Arduino IDE or `arduino-cli`.

If you have never used an ESP32 before, follow the Arduino IDE path first — it
hides the most setup. Move to `arduino-cli` once you want a one-line build/flash
loop.

---

## 1. Install the USB-serial driver (macOS)

The ESP32 DevKit uses either a CP210x or CH340 USB-serial chip. macOS may not
have the driver out of the box.

1. Plug the ESP32 in with a known-good **data** USB-C cable (charge-only cables
   are a common time sink).
2. In a terminal, list serial devices:

   ```sh
   ls /dev/cu.*
   ```

3. If you see something like `/dev/cu.usbserial-XXXX` or `/dev/cu.SLAB_USBtoUART`,
   you're good. If not:
   - **CP210x (Silicon Labs):** download the driver from
     <https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers>
   - **CH340 (WCH):** <https://www.wch-ic.com/downloads/CH34XSER_MAC_ZIP.html>
4. After install, replug the board and re-run `ls /dev/cu.*`. Note the device
   path; you'll need it below.

---

## 2. Install Arduino IDE + ESP32 board support

1. Download the Arduino IDE 2.x: <https://www.arduino.cc/en/software>
2. Open **Settings** (`Cmd+,`).
3. In **Additional boards manager URLs**, paste:

   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

4. Open **Tools -> Board -> Boards Manager**, search "esp32", install
   **esp32 by Espressif Systems** (latest 3.x).
5. Open **Tools -> Board -> esp32**, pick **ESP32 Dev Module**.
6. Open **Tools -> Port**, pick the `/dev/cu.usbserial-*` (or `SLAB_USBtoUART`)
   device.

Recommended Tools settings (defaults are mostly fine):

| Setting           | Value                          |
|-------------------|--------------------------------|
| Board             | ESP32 Dev Module               |
| Upload Speed      | 921600                         |
| CPU Frequency     | 240MHz                         |
| Flash Frequency   | 80MHz                          |
| Flash Mode        | QIO                            |
| Flash Size        | 4MB (32Mb)                     |
| Partition Scheme  | Default 4MB with spiffs        |
| Core Debug Level  | None (raise to Verbose if BLE / Wi-Fi misbehave) |

---

## 3. Install the libraries

In Arduino IDE: **Tools -> Manage Libraries** (`Cmd+Shift+I`), then install:

- **Adafruit SSD1306**
- **Adafruit GFX Library** (pulled in as a dep, confirm it lands)
- **NimBLE-Arduino** (only needed once Phase 4 BLE work starts)

The TDC7200 driver is in-tree (`firmware/openscan/tdc.cpp`) — no library to install.

---

## 4. Open the sketch

Arduino IDE expects the sketch folder name to match the `.ino` file name. The
repo already follows this:

```text
firmware/openscan/openscan.ino   <- open this file
```

**File -> Open**, navigate to `firmware/openscan/openscan.ino`, open it. The IDE
will load all the `.h` / `.cpp` siblings as tabs.

---

## 5. Build and flash

1. Confirm **Tools -> Port** still points at the ESP32.
2. Click **Verify** (checkmark) — the first build pulls in the ESP32 toolchain
   and takes a few minutes. Subsequent builds are fast.
3. Click **Upload** (right arrow). On most ESP32 DevKits, the bootloader is
   entered automatically. If you see `Failed to connect to ESP32: ...`:
   - Hold **BOOT** on the board.
   - Press and release **EN** (reset).
   - Release **BOOT**.
   - Re-click **Upload**.
4. Open **Serial Monitor** (`Cmd+Shift+M`), set baud to **115200**. You should
   see `openscan boot` followed by `ready` once the OLED comes up.

---

## 6. Running the bench self-tests

Each Phase 1 subsystem has a standalone sketch under `firmware/tests/`. Open
each one in turn (one sketch per Arduino IDE window) and flash:

| Sketch                       | Verifies                                          |
|------------------------------|---------------------------------------------------|
| `selftest_display.ino`       | OLED wiring + I2C address                         |
| `selftest_button.ino`        | Trigger button + debounce                         |
| `selftest_tdc.ino`           | TDC7200 SPI bring-up with software START/STOP     |
| `selftest_pulser.ino`        | Avalanche pulser GPIO trigger (SAFETY: see file)  |

Do not skip these. They are the firmware mirror of the Phase 1 bench-test plan
in `rangefinder_build.md`. Bringing the full sketch up first is how parts get
bricked.

**Pulser self-test safety:** `selftest_pulser.ino` fires the laser at 1 Hz.
Before running it, confirm a beam dump or non-reflective wall is in the beam
path, OD4+ 905 nm goggles are on, and there is nothing reflective in the room.

---

## 7. (Optional) `arduino-cli` workflow

Once the IDE flow works, `arduino-cli` is faster for iteration.

```sh
# install
brew install arduino-cli

# one-time setup
arduino-cli config init
arduino-cli config add board_manager.additional_urls \
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library"

# build + flash (replace the port with your device)
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/openscan
arduino-cli upload  --fqbn esp32:esp32:esp32 \
  -p /dev/cu.usbserial-XXXX firmware/openscan

# serial monitor
arduino-cli monitor -p /dev/cu.usbserial-XXXX -c baudrate=115200
```

---

## Troubleshooting

**`Failed to connect to ESP32: Timed out waiting for packet header`**
Hold BOOT, tap EN, release BOOT, retry upload. Try a different USB cable
(charge-only cables look fine but won't enumerate). Lower upload speed to 115200
in Tools.

**OLED stays blank**
Check I2C wiring (SDA=21, SCL=22 by default in `config.h`). Run an I2C scanner
sketch and confirm address `0x3C`. Some SSD1306 modules ship as `0x3D` — update
`config.h`.

**Garbled serial output**
Wrong baud rate. Set Serial Monitor to **115200**.

**Sketch compiles but won't fit**
You probably enabled BLE + Wi-Fi + Verbose debug at once. Drop debug level back
to None, and select a partition scheme with more app space if needed.

**`#include "config.h"` not found**
The IDE only sees siblings of the `.ino` file. Make sure you opened
`firmware/openscan/openscan.ino` directly, not a copy elsewhere.
