# DIY Laser Rangefinder Build

A 905nm pulsed time-of-flight rangefinder targeting golf-distance ranging (200+ yards) on a non-cooperative target like a flagstick. Companion project to the OpenLaunch / LaunchLab DIY launch monitor.

## Architecture

```
[ESP32] ──TRIG──> [Avalanche pulser] ──> [905nm laser diode] ──> [Collimator lens] ──> Target
   │                                                                                     │
   │                                                                                     │ (return photons)
   │                                                                                     ▼
   │                                                                          [Receive lens]
   │                                                                                     │
   │                                                                                     ▼
   │              [TDC7200] <── STOP ── [Comparator] <── [RF amp] <── [SiPM + bias supply]
   │                  │
   └── SPI ───────────┘
   │
   └── I2C ──> [OLED display]
   └── GPIO ──> [Trigger button]
```

**Operating principle:** ESP32 fires a sub-100ns laser pulse and simultaneously starts the TDC7200 timer. The pulse hits the target, scatters, and a tiny fraction of photons return to the SiPM detector. The SiPM produces a fast electrical pulse, which is amplified, thresholded by a comparator, and stops the TDC. Distance = (time × speed of light) / 2.

**Timing requirement:** ~1 yard accuracy needs ~3ns timing resolution. The TDC7200 provides ~55ps resolution, well within spec.

---

## Bill of Materials

### Digikey (~$165)

| Part | Digikey # | Qty | Price | Notes |
|---|---|---|---|---|
| SPL PL90AT03 laser diode | 475-SPLPL90AT03-ND | 2 | $19.09 ea | 905nm pulsed, 75W peak max — drive at 10–15A peak (~25–30W optical) for safety margin. Spare included. |
| MIKROE-4770 TDC Click | 1471-MIKROE-4770-ND | 1 | $18.00 | TDC7200 breakout, ~55ps resolution |
| TLV3501AIDR comparator | 296-43611-1-ND | 1 | $3.32 | 4.5ns prop delay, sets discriminator threshold |
| MicroFC-SMTPA-60035 SiPM | MICROFC-SMTPA-60035-GEVBOS-ND | 1 | $100.67 | 6×6mm active area, ~27V bias, fast-output pin |
| GALI-5+ RF amplifier | 3157-GALI-5+CT-ND | 1 | $5.10 | +20dB, DC-4GHz, SiPM fast-output amplification |

### Amazon (~$124)

| Part | Qty | Price | Notes |
|---|---|---|---|
| ESP32 DevKit (Lonely Binary 3-pack) | 1 | $29.99 | Reputable seller, better USB-serial than clones |
| OCR 180pc tactile button kit | 1 | $7.99 | 6×6mm momentary buttons, plenty of spares |
| FainWan XL6009 boost converter (2-pack) | 1 | $13.99 | Set to 28V output before connecting SiPM |
| TEKOWEE 18650 cells (4-pack) | 1 | $19.99 | ⚠️ Unverified capacity rating — see safety notes |
| AEDIKO TP4056 + holder (5-pack) | 1 | $7.99 | USB-C charging, includes DW01 protection chip |
| ALLECIN 2N3904 transistors (200pc) | 1 | $7.59 | Avalanche pulser; expect to burn through several |
| PATIKIL 50mm convex lens (4-pack) | 1 | $9.99 | Receive optic — basic, may upgrade later |
| CAY046 aspheric collimator | 1 | $11.00 | Transmit optic, NA 0.40, EFL 4.60mm |
| Hosyond SSD1306 OLED (5-pack) | 1 | $14.99 | 0.96" 128×64 I2C display |

### Already on hand
- Breadboard, jumper wires
- Resistor and capacitor assortment

### Optional / future
- Custom PCB for SiPM bias + amp + comparator (JLCPCB/OSHPark, ~$5)
- Panel-mount momentary button for enclosure (~$8)
- Thorlabs LA1131 AR-coated 50mm lens (~$30) if range-limited

**Total: ~$290**

---

## Build Plan

### Phase 1: Subsystem bench testing (do before integration)

Each subsystem gets validated independently. Bringing it all up at once is how you brick parts.

#### 1a. Bias supply
- Wire XL6009 to 18650 + holder
- Adjust trim pot to **28.0V** with multimeter on output
- Add RC filter: 1kΩ + 10µF tantalum + 100nF ceramic at output
- Verify voltage holds steady under no load
- **Do not connect SiPM yet**

#### 1b. Laser pulser
- Build 2N3904 avalanche pulser circuit on breadboard
- Use a **photodiode + oscilloscope** (or a fast scope alone) to verify pulse shape
- Target: ~50ns pulse width, clean rising edge
- **Aim into a beam dump or far wall** — never at any reflective surface, never at eyes
- Tune pulse current to ~10–15A peak (measure with current-viewing resistor or scope probe across known impedance)
- Record pulse repetition rate — keep duty cycle < 0.1% for thermal + eye-safety margin

#### 1c. SiPM + amp + comparator chain
- Power SiPM at 27V via the RC-filtered bias supply
- AC-couple fast output through 100pF cap to GALI-5+ input
- GALI-5+ output → TLV3501 comparator
- Comparator threshold set by resistor divider, start ~10mV above noise floor
- Test with a slow LED pulse (visible from a phone flashlight) — should see clean digital pulses on comparator output
- **Bias must be OFF whenever soldering or rewiring near the SiPM** — one mistake = $100 part

#### 1d. ESP32 + TDC + display
- Wire TDC Click to ESP32 over SPI
- Wire SSD1306 OLED over I2C
- Wire button to GPIO with internal pull-up enabled
- Run a "fake measurement" test: software-generated START and STOP pulses to verify TDC reads correctly
- Display should update on button press

### Phase 2: Optical alignment

Once electronics work, the optics determine whether you can actually hit a flag at distance.

- Mount laser diode in a 3D-printed or machined holder with the CAY046 collimator
- **Beam divergence target:** < 5 mrad (gives ~1m beam diameter at 200m). Tighter is better.
- Use an IR viewer card or phone camera (silicon CMOS sees 905nm) to visualize the beam
- Mount receive lens with SiPM at its focal plane (50mm behind lens)
- Boresight transmit and receive paths to within ~1° at distance — at close range they can miss each other entirely (parallax)
- Add a simple iron sight or red-dot atop the housing for aiming

### Phase 3: System integration

- Combine all subsystems
- Calibrate timing offset: point at a target at known distance (e.g., 10m), record the TDC reading, subtract this baseline from all future measurements
- Confirm linearity at multiple known distances (10m, 50m, 100m)
- Tune comparator threshold for best range vs false-trigger tradeoff
- Measure noise floor — single-photon dark counts will set your minimum detectable signal

### Phase 4: Field testing
- Test on a flagstick at a driving range, starting at 50 yards and working out
- Compare against a known-good commercial unit (Bushnell, Garmin) for ground truth
- Log measurements over BLE to phone for analysis
- Iterate optics if range is limited

---

## Critical Safety Notes

### Eye safety
- 905nm is **invisible** — no blink reflex. Treat the laser as live whenever powered, even if you can't see anything.
- Class 1 eye-safe operation requires keeping average power low. The math: peak power × pulse width × repetition rate = average power.
- At 25W peak, 50ns pulse, 1kHz rep rate: average power = 25 × 50e-9 × 1000 = 1.25 mW. Class 1 limit at 905nm is ~0.78 mW for accessible emission, but tight collimation (small beam divergence) can be considered, and pulsed exposure has separate MPE calculations.
- **Run the eye-safety math against IEC 60825-1 before pointing the device at any human target zone**
- Don't point at people, mirrors, glass, water, or polished metal
- Wear OD4+ 905nm-rated safety goggles during bench testing

### Battery safety
- Charge 18650s on a non-flammable surface (ceramic plate, cookie sheet)
- Don't leave charging unattended on first few cycles
- If a cell warms up during charge or discharge, stop immediately
- TEKOWEE cells are unverified — capacity rating may be inflated. For long-term reliability, replace with Samsung 30Q or Molicel P26A from 18650batterystore.com.

### SiPM survival
- Keep bias OFF when soldering, rewiring, or probing
- One overvoltage event = dead $100 part
- Always bring up bias supply slowly, verify voltage with meter before connecting

---

## Known Risks / Open Questions

1. **Beam divergence with consumer-grade collimator** — CAY046 may not give tight enough beam for 200+ yard ranging. May need to upgrade to multi-element collimator.
2. **Receive optic quality** — PATIKIL "trial set" lens is uncoated and may scatter. Likely the first upgrade if range is disappointing.
3. **SiPM dark count rate at room temp** — sets the false-trigger floor. May need to add coincidence detection (require N detections in a window) to suppress.
4. **Ambient light rejection** — direct sunlight on the receive aperture will saturate the SiPM. A 905nm bandpass filter (~$30 from Edmund Optics) is a likely v2 addition.
5. **Avalanche pulser jitter** — 2N3904 avalanche pulsers have ns-scale timing jitter that adds to range error. May need to upgrade to a GaN FET driver (e.g., EPC9126HC) for v2.

---

## Next Steps for Agent

In order of priority:

1. **Place the orders** — Digikey cart and Amazon cart are finalized (see BOM above). Total ~$290.
2. **While waiting for parts:**
   - Read the OSRAM SPL PL90_3 Range Finder application note (PDF on Digikey product page) — it documents the avalanche pulser circuit topology and expected pulse shapes
   - Read the Onsemi MicroFC datasheet, focusing on the fast-output pin behavior and bias voltage / overvoltage specifications
   - Read the TI TDC7200 datasheet, focusing on Mode 1 (short-range) vs Mode 2 (long-range) operation and how the START/STOP edge timing is registered
   - Sketch the avalanche pulser schematic with component values for the 2N3904 and confirm against the Osram app note
3. **When parts arrive:**
   - Follow the Phase 1 subsystem bring-up procedure in order
   - Do not skip the bench testing phase even if eager to integrate
   - Document oscilloscope captures of the laser pulse shape and SiPM output for content / build log
4. **Content strategy** — the avalanche pulser scope captures and the "how do you measure something that takes a billionth of a second" hook are the strongest early content beats. Film bench testing as it happens, not in retrospect.

---

## Reference Datasheets

- **SPL PL90AT03**: ams-OSRAM, 905nm 75W pulsed laser diode
- **MicroFC-60035**: Onsemi C-Series SiPM, 6×6mm
- **TDC7200**: Texas Instruments, time-to-digital converter
- **TLV3501**: Texas Instruments, 4.5ns rail-to-rail comparator
- **GALI-5+**: Mini-Circuits, DC-4GHz MMIC amplifier
- **XL6009**: XLSEMI, DC-DC boost converter
- **TP4056**: NanJing Top Power, single-cell Li-ion charger
- **2N3904**: NPN BJT, used in avalanche breakdown mode here
