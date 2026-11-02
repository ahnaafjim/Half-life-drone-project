/*
  bringup.ino - sensor bring-up test for the ESP32 DIY Drone Flight Controller (Rev 1)

  What it proves, in order:
    1. Serial over USB-C / CH340E works
    2. I2C bus (GPIO21/22) is alive: scans and lists every device that answers
    3. LSM6DS3TR-C answers at 0x6A with WHO_AM_I = 0x6A, then streams accel + gyro
    4. BMP388 answers at 0x76 with chip ID = 0x50, then streams pressure, altitude, temperature
    5. IMU INT1 / INT2 lines (GPIO19 / GPIO23) are wired: data-ready is routed to them and their level is printed

  A failed sensor does NOT stop the sketch, so one bad solder joint doesn't hide the other sensor.
  Open the Serial Monitor at 115200 baud.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_BMP3XX.h>
#include "pins.h"

#define SEALEVELPRESSURE_HPA 1013.25f   // change to your local value for a true altitude
#define LOOP_PERIOD_MS       200

Adafruit_LSM6DS3TRC imu;
Adafruit_BMP3XX     baro;

bool imuOk  = false;
bool baroOk = false;

// Read one register straight over I2C (no library) so the check is independent of the drivers.
static int readReg(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom((int)addr, 1) != 1) return -1;
  return Wire.read();
}

static void scanI2C() {
  Serial.println("[I2C] Scanning 0x01..0x7E");
  int found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found 0x%02X", a);
      if (a == ADDR_IMU)  Serial.print("  (expected: LSM6DS3TR-C)");
      if (a == ADDR_BARO) Serial.print("  (expected: BMP388)");
      Serial.println();
      found++;
    }
  }
  if (found == 0) Serial.println("  nothing found - check 3V3 rail, SDA/SCL and pull-ups");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 FC Rev 1 - sensor bring-up ===");

  pinMode(PIN_IMU_INT1, INPUT);
  pinMode(PIN_IMU_INT2, INPUT);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(I2C_FREQ_HZ);
  scanI2C();

  // ---- IMU ----
  int who = readReg(ADDR_IMU, 0x0F);                       // WHO_AM_I
  Serial.printf("[IMU ] WHO_AM_I = 0x%02X (expect 0x6A)\n", who < 0 ? 0xFF : who);
  imuOk = imu.begin_I2C(ADDR_IMU, &Wire);
  if (imuOk) {
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_8_G);
    imu.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
    imu.setGyroDataRate(LSM6DS_RATE_104_HZ);
    imu.configIntOutputs(false, false);                    // active-high, push-pull
    imu.configInt1(false, false, true);                    // INT1 = accel data-ready
    imu.configInt2(false, true, false);                    // INT2 = gyro data-ready
    Serial.println("[IMU ] LSM6DS3TR-C OK");
  } else {
    Serial.println("[IMU ] FAIL - LSM6DS3TR-C not found at 0x6A");
  }

  // ---- Barometer ----
  int id = readReg(ADDR_BARO, 0x00);                       // CHIP_ID
  Serial.printf("[BARO] CHIP_ID = 0x%02X (expect 0x50)\n", id < 0 ? 0xFF : id);
  baroOk = baro.begin_I2C(ADDR_BARO, &Wire);
  if (baroOk) {
    baro.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    baro.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    baro.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    baro.setOutputDataRate(BMP3_ODR_50_HZ);
    Serial.println("[BARO] BMP388 OK");
  } else {
    Serial.println("[BARO] FAIL - BMP388 not found at 0x76");
  }

  Serial.printf("\nSUMMARY: IMU %s | BARO %s\n\n", imuOk ? "PASS" : "FAIL", baroOk ? "PASS" : "FAIL");
}

void loop() {
  if (imuOk) {
    // Sample the interrupt pins first: a high level means a fresh sample is waiting.
    int i1 = digitalRead(PIN_IMU_INT1);
    int i2 = digitalRead(PIN_IMU_INT2);

    sensors_event_t accel, gyro, temp;
    imu.getEvent(&accel, &gyro, &temp);                    // reading clears data-ready

    Serial.printf("ACC[m/s2] %7.2f %7.2f %7.2f | GYR[rad/s] %7.3f %7.3f %7.3f | INT1=%d INT2=%d",
                  accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
                  gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, i1, i2);
  } else {
    Serial.print("IMU n/a");
  }

  if (baroOk && baro.performReading()) {
    Serial.printf(" | P=%.2f hPa  Alt=%.2f m  T=%.2f C\n",
                  baro.pressure / 100.0f, baro.readAltitude(SEALEVELPRESSURE_HPA), baro.temperature);
  } else {
    Serial.println(" | BARO n/a");
  }

  delay(LOOP_PERIOD_MS);
}
