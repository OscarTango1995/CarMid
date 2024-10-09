#ifndef LOGGER_H
#define LOGGER_H
#include <Arduino.h> 

void initLogging();
void logError(const String &errorMessage);
void logInfo(const String &infoMessage);
void logAvg(const String &AvgMessage);

#endif // LOGGER_H
