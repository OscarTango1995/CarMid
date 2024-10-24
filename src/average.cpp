#include <Arduino.h>
#include "average.h"
#include "sd.h"
#include "fuel_level.h"

const int MAX_SIZE = 10;
static bool entriesFull = false;

Average calculateAverageFuelConsumption()
{
    Average lastAvg = readLastAverageData();
    Total newTotals = readFuelDistanceCSV();

    if (newTotals.totalDistance == 0)
    {
        Serial.println("Error: Total distance is zero, cannot calculate average.");
        return Average();
    }

    float newFuelUsed = newTotals.totalFuel; // fuel in literes

    Average avg;
    avg.average = (lastAvg.distanceTraveled + newTotals.totalDistance) / (lastAvg.fuelUsed + newFuelUsed);
    avg.distanceTraveled = lastAvg.distanceTraveled + newTotals.totalDistance;
    avg.fuelUsed = lastAvg.fuelUsed + newFuelUsed;
    avg.fuelRemaining = getFuelAvailable();
    avg.dte = getFuelAvailable() * avg.average;

    // write average to csv
    bool isWritten = writeAvgToCSV(avg);
    if (isWritten)
    {
        resetLineCount();
        resetFuelDistanceCSV();
        resetArray();
        return avg;
    }
    else
    {
        Serial.println("Failed to write Average Data to csv");
        return Average();
    }
}

// Function to store values in arrays and check if they are full
void logAverageData(float fuelFlow, double distance)
{
    writeFuelDistanceToCSV(fuelFlow, distance);
    writeLineCount();
    int lineCount = readLineCount();

    if (lineCount > 9)
    {
        entriesFull = true;
    }
    else
    {
        entriesFull = false;
    }
}

bool isArrayFull()
{
    return entriesFull;
}

void resetArray()
{
    entriesFull = false;
}

Average resetTrip()
{
    entriesFull = false;
    Average avg;
    avg.average = 0.0;
    avg.distanceTraveled = 0.0;
    avg.fuelRemaining = 0.0;
    avg.fuelUsed = 0.0;
    avg.dte = 0.0;
    resetFuelDistanceCSV();
    resetLineCount();
    writeAvgToCSV(avg);

    return avg;
}