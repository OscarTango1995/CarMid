#include <Arduino.h>
#include "fuel_level.h"

#define VOLTAGE_PIN 32             // Use ADC pin (GPIO32 in this case)
#define REF_VOLTAGE 3.3            // Reference voltage (3.3V for ESP32)
#define ADC_MAX 4095               // Maximum ADC value for 12-bit resolution

const float tankCapacity = 35.0; // Total tank capacity in liters

void initFuelLevelSensor()
{
    pinMode(VOLTAGE_PIN, INPUT);
}

float readVoltage()
{
    float voltage = 0.00;
    for (u_int8_t i = 0; i < 5; i++)
    {
        float adcValue = analogRead(VOLTAGE_PIN);      // Read ADC value
        voltage += (adcValue * REF_VOLTAGE) / ADC_MAX; // Convert ADC value to voltage
        delay(10);
    }

    return voltage / 5;
}

float getFuelAvailable()
{
    float vin = readVoltage();
    float fuelPercentage = getFuelPercentage(vin);
    float fuelLiters = (fuelPercentage / 100.0) * tankCapacity;

    return fuelLiters;
}

float getFuelPercentage(float voltage)
{
    // Define voltage and percentage pairs
    //                          E,1/4,1/2,3/4,F
    float voltageLevels[] = {1.14, 1.00, 0.85, 0.42, 0.12};
    float fuelPercentages[] = {0, 25, 50, 75, 100};

    for (int i = 0; i < 4; i++)
    {
        // Find the range where the voltage falls
        if (voltage <= voltageLevels[i] && voltage >= voltageLevels[i + 1])
        {
            // Interpolate between the bounds
            return fuelPercentages[i] + ((voltage - voltageLevels[i]) / (voltageLevels[i + 1] - voltageLevels[i])) * (fuelPercentages[i + 1] - fuelPercentages[i]);
        }
    }

    // Return 0 if below empty or 100 if above full
    return (voltage > voltageLevels[0]) ? 0 : 100;
}
