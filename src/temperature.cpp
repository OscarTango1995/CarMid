#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.h"
#include "buzzer.h"
#include "elm.h"

DHT dht(15, DHT21); // GPIO pin,Sensor Type
OneWire oneWire(4); // OneWire bus on GPIO pin 4
DallasTemperature sensors(&oneWire);

// Function to initialize the Temperaure sensors
void initTempSensors()
{
    dht.begin();     // Initialize the DHT sensor
    sensors.begin(); // Initialize the DS18B20 sensor
}

float readOutsideTemperature()
{
    sensors.requestTemperatures();
    return sensors.getTempCByIndex(0);
}

InsideTempHumidity readInsideTemperature()
{
    InsideTempHumidity result;
    result.temperature = dht.readTemperature();
    result.humidity = dht.readHumidity();
    return result;
}

Temperatures getTemperatures()
{
    InsideTempHumidity insideTemp = readInsideTemperature();
    delay(750);

    float outsideTemp = readOutsideTemperature();
    delay(750);

    Temperatures temp;
    temp.humidity = insideTemp.humidity - 16.0; // For calibration
    temp.iat = insideTemp.temperature + 0.0;    // For calibration
    temp.oat = outsideTemp + 1.0;               // For calibration
    return temp;
}