#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "altitude.h"

Adafruit_BMP280 bmp;

#define SEALEVELPRESSURE_HPA (1013.25)

float temperatureCalibration = 0.0;


void initAltitude()
{

  if (!bmp.begin(0x76))
  {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1)
      ;
  }

  bmp.setSampling(Adafruit_BMP280::MODE_SLEEP,       
                  Adafruit_BMP280::SAMPLING_X2,      
                  Adafruit_BMP280::SAMPLING_X16,     
                  Adafruit_BMP280::FILTER_X16,       
                  Adafruit_BMP280::STANDBY_MS_4000); 
}

Altitude getAltitude()
{
  bmp.reset();
  initAltitude();
  delay(750);

  bmp.takeForcedMeasurement();
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Altitude result;
  result.altitude = altitude;
  result.pressure = pressure;
  result.temperature = temperature;
  return result;
}
