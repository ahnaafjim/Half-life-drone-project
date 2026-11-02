# Firmware: sensor bring-up

`bringup/bringup.ino` is a hardware bring-up test, not flight code. It checks that the board's power rails, USB-UART, I2C bus, IMU, barometer and IMU interrupt lines all work.

## What it does
1. Starts Serial at 115200 baud.
2. Scans the I2C bus (SDA `GPIO21`, SCL `GPIO22`, 400 kHz) and lists every address that answers.
3. Reads the **LSM6DS3TR-C** WHO_AM_I register (expects `0x6A`) and the **BMP388** chip ID (expects `0x50`), directly over I2C.
4. Starts both drivers and prints `PASS` / `FAIL` per sensor. A failure doesn't stop the sketch.
5. Every 200 ms prints accel (m/s²), gyro (rad/s), pressure (hPa), altitude (m), temperature (°C) and the levels of `INT1` / `INT2`.

## Pin configuration
Defined in [`bringup/pins.h`](bringup/pins.h).

| Function | GPIO | Notes |
| :--- | :--- | :--- |
| I2C SDA | 21 | 4.7 kΩ pull-up (R7) |
| I2C SCL | 22 | 4.7 kΩ pull-up (R6) |
| IMU INT1 / INT2 | 19 / 23 | LSM6DS3TR-C, set to data-ready in this sketch |
| Motors 1–4 | 25 / 26 / 27 / 32 | Not driven by this sketch |
| BATT_SENSE / CURR_SENSE | 34 / 35 | Not used by this sketch |
| BOOT | 0 | Strapping pin |
| UART0 | 1 (TX) / 3 (RX) | To CH340E |

I2C addresses: IMU `0x6A`, BMP388 `0x76`.

## Dependencies

| Item | Version |
| :--- | :--- |
| Arduino-ESP32 core | 2.0.17 (3.x should also work) |
| Board | ESP32 Dev Module |
| Adafruit LSM6DS | 4.7.4 |
| Adafruit BMP3XX Library | 2.1.6 |
| Adafruit Unified Sensor, Adafruit BusIO | latest (installed automatically with the libraries above) |

### Arduino IDE
1. Install the ESP32 board package (Boards Manager, "esp32 by Espressif Systems").
2. Install the libraries above (Library Manager) and accept "install all dependencies".
3. Open `firmware/bringup/bringup.ino` (the folder name must match the sketch name, so don't rename one without the other).
4. Select **ESP32 Dev Module**, the right COM port, then upload.

### PlatformIO
```
cd firmware
pio run -t upload
pio device monitor
```

## Flashing
The CH340E has no DTR/RTS lines, so the board can't reset itself into download mode. Do this by hand:
1. Connect USB-C only (no external power on `B+`).
2. Press **Upload**. When the IDE shows `Connecting......`, hold **BOOT**, tap **RESET**, then release **BOOT**.
3. After upload finishes, tap **RESET** to start the sketch.

## Expected output (healthy board)
```
=== ESP32 FC Rev 1 - sensor bring-up ===
[I2C] Scanning 0x01..0x7E
  found 0x6A  (expected: LSM6DS3TR-C)
  found 0x76  (expected: BMP388)
[IMU ] WHO_AM_I = 0x6A (expect 0x6A)
[IMU ] LSM6DS3TR-C OK
[BARO] CHIP_ID = 0x50 (expect 0x50)
[BARO] BMP388 OK

SUMMARY: IMU PASS | BARO PASS
```
Then a line every 200 ms. With the board flat and still, accel Z should read about +9.8 m/s², gyro values near 0, and pressure roughly 1000 hPa (varies with weather/altitude).

## Troubleshooting
| Symptom | Likely cause |
| :--- | :--- |
| Upload hangs on `Connecting...` | Didn't use the BOOT + RESET sequence |
| Scan finds nothing | 3V3 rail down, SDA/SCL shorted or swapped, missing pull-ups |
| Only `0x6A` shows up | BMP388 soldering (LGA-10 pads) |
| IMU found, INT1/INT2 always 0 | Interrupt trace/solder problem on IO19 / IO23 |
