#ifndef SD_H
#define SD_H
#include <Arduino.h> 
#include"average.h"

struct Total{
    float totalFuel;
    float totalDistance;
};
struct GPS{
    double latitude;
    double longitude;
    double distance;
};
void initSDCard(bool firstTime);
bool writeAvgToCSV(Average avg);
void initAverageFile();
void initErrorFile();
void writeError(String resetReason);
void initOBDFile();
void writeOBDToCSV(float engineLoad, uint8_t manifoldPressure, float throttlePosition, float rpm, float fuelFlow);
Average readLastAverageData();
void writeFuelDistanceToCSV(float fuelFlow, float distance);
void initFuelFile();
void resetAverageFile();
void resetAverageFile();
void writeLineCount();
int readLineCount();
void initLineCountFile();
Total readFuelDistanceCSV();
void resetLineCount();
void resetFuelDistanceCSV();
void deleteAllFiles();
void initGPSFile();
void resetGPSFile();
GPS readLastCoordinates();
void writeGPSToCSV(double latitude,double longitude,double distance);

#endif // LOGGER_H
