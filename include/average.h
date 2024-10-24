#ifndef AVERAGE_H
#define AVERAGE_H
#include <Arduino.h>

struct Average
{
    float average;
    float distanceTraveled;
    float fuelUsed;
    float fuelRemaining;
    float dte;
};

Average calculateAverageFuelConsumption();
void logAverageData(float fuelFlow, double distance);
bool isArrayFull();
void resetArray();
Average resetTrip();

#endif // AVERAGE_H
