// pins.h - GPIO map for the ESP32 DIY Drone Flight Controller (Rev 1)
// Source of truth: schematics/SCH_New-Project_2026-10-13.json and the PCB file in "pcb design/".
// Net names in the PCB file are "U2_<module pin>", e.g. U2_33 = ESP32-WROOM-32E pin 33 = GPIO21.
#pragma once

// ---- I2C bus shared by IMU, barometer and the external 4-pin header ----
#define PIN_I2C_SDA      21   // module pin 33, 4.7k pull-up to 3V3
#define PIN_I2C_SCL      22   // module pin 36, 4.7k pull-up to 3V3
#define I2C_FREQ_HZ      400000

// ---- Sensor I2C addresses (set by address pins on the PCB) ----
#define ADDR_IMU         0x6A // LSM6DS3TR-C, SA0/SDO tied to GND
#define ADDR_BARO        0x76 // BMP388, SDO tied to GND (0x77 if SDO were high)

// ---- IMU interrupt lines ----
#define PIN_IMU_INT1     19   // module pin 31, LSM6DS3TR-C INT1
#define PIN_IMU_INT2     23   // module pin 37, LSM6DS3TR-C INT2

// ---- Motor / ESC signal headers (2.54 mm) ----
#define PIN_MOTOR1       25   // module pin 10
#define PIN_MOTOR2       26   // module pin 11
#define PIN_MOTOR3       27   // module pin 12
#define PIN_MOTOR4       32   // module pin 8

// ---- Analog inputs (input-only ADC1 pins, header only, external divider needed) ----
#define PIN_BATT_SENSE   34   // module pin 6
#define PIN_CURR_SENSE   35   // module pin 7

// ---- Buttons ----
#define PIN_BOOT         0    // KEY2, strapping pin, pulled up. Hold low at reset = download mode
// EN (reset) is KEY1 and is not a GPIO.

// ---- USB-UART (CH340E) ----
// UART0: GPIO1 = TX0, GPIO3 = RX0 (used by Serial / flashing, via level shifter)
