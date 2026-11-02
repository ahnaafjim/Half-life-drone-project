#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_BMP3XX.h>

// Sea level pressure standard in hPa (adjust for your local weather/calibration)
#define SEALEVELPRESSURE_HPA (1013.25)

// I2C Pin definitions from schematic
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Sensor instances
Adafruit_LSM6DS3TRC lsm6ds;
Adafruit_BMP3XX bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial monitor connection
  }

  Serial.println("\n--- ESP32 Sensor Initialization ---");

  // Initialize I2C bus: SDA = GPIO 21, SCL = GPIO 22
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // 1. Initialize LSM6DS3TR-C (Address: 0x6A per schematic SA0 -> GND)
  if (!lsm6ds.begin_I2C(0x6A, &Wire)) {
    Serial.println("Failed to find LSM6DS3TR-C IMU at 0x6A! Check soldering/wiring.");
    while (1) {
      delay(100);
    }
  }
  Serial.println("LSM6DS3TR-C IMU initialized successfully!");

  // Configure LSM6DS3TR-C ranges
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_8_G);
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ);
  lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ);

  // 2. Initialize BMP388 Barometer (Check 0x77, fallback to 0x76)
  bool bmp_found = bmp.begin_I2C(0x77, &Wire);
  if (!bmp_found) {
    bmp_found = bmp.begin_I2C(0x76, &Wire);
  }

  if (!bmp_found) {
    Serial.println("Failed to find BMP388 Barometer! Check soldering/wiring.");
    while (1) {
      delay(100);
    }
  }
  Serial.println("BMP388 Barometer initialized successfully!");

  // Configure BMP388 oversampling and filter settings
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  delay(200);
  Serial.println("Starting measurements...\n");
}

void loop() {
  // Read IMU events
  sensors_event_t accel, gyro, temp;
  lsm6ds.getEvent(&accel, &gyro, &temp);

  // Read Barometer
  bool bmp_ok = bmp.performReading();

  // 1. Acceleration [m/s^2]
  Serial.print("Accel [m/s^2]: X=");
  Serial.print(accel.acceleration.x, 2);
  Serial.print(" Y=");
  Serial.print(accel.acceleration.y, 2);
  Serial.print(" Z=");
  Serial.print(accel.acceleration.z, 2);

  // 2. Gyroscope [rad/s]
  Serial.print(" | Gyro [rad/s]: X=");
  Serial.print(gyro.gyro.x, 2);
  Serial.print(" Y=");
  Serial.print(gyro.gyro.y, 2);
  Serial.print(" Z=");
  Serial.print(gyro.gyro.z, 2);

  // 3. Barometer Readings (Pressure, Altitude, Temp)
  if (bmp_ok) {
    Serial.print(" | Pressure: ");
    Serial.print(bmp.pressure / 100.0, 2); // Convert Pa to hPa
    Serial.print(" hPa");

    Serial.print(" | Alt: ");
    Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA), 2);
    Serial.print(" m");

    Serial.print(" | Baro Temp: ");
    Serial.print(bmp.temperature, 2);
    Serial.println(" °C");
  } else {
    Serial.println(" | BMP388 read failed");
  }

  delay(200);
}