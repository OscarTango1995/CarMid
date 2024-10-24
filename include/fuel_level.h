#ifndef FUEL_LEVEL_H
#define FUEL_LEVEL_H

// Function to initialize the voltage sensor
void initFuelLevelSensor();
float readVoltage();
float getFuelAvailable();
float getFuelPercentage(float voltage);

#endif // VOLTAGE_SENSOR_H
