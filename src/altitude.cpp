#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include"altitude.h"

// Create a BMP280 object
Adafruit_BMP280 bmp;

// Define sea-level pressure (in hPa) for your location (approximate)
#define SEALEVELPRESSURE_HPA (1015.0)

float temperatureCalibration=0.0;

// Function to initialize the Temperaure sensors
void initAltitude() {
  if (!bmp.begin(0x76)) {  // Change to 0x77 if needed
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  // Configure BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,      // Operating Mode
                  Adafruit_BMP280::SAMPLING_X2,      // Temp. oversampling
                  Adafruit_BMP280::SAMPLING_X16,     // Pressure oversampling
                  Adafruit_BMP280::FILTER_X16,       // Filtering
                  Adafruit_BMP280::STANDBY_MS_500);  // Standby time

}

Altitude getAltitude(){
    bmp.takeForcedMeasurement(); // Force the sensor to take a measurement

    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;  // Convert Pa to hPa
    float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    delay(750);

    Altitude result;
    result.altitude=altitude;
    result.pressure=pressure;
    result.temperature=temperature;
    return result;
}
