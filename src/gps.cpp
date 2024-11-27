#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "gps.h"
#include "sd.h"

// Create GPS object
TinyGPSPlus gps;

// Use HardwareSerial to communicate with the GPS module
HardwareSerial gpsSerial(1); // UART1
int RXPin = 17;              // Connect this to TX of GPS
int TXPin = 5;               // Connect this to RX of GPS (if needed)

// Variable to hold the last distance traveled during the current minute
double lastDistance = 0.0;
int distanceOffset = 500.0;

const long TIMEZONE_OFFSET = 5 * 3600; // 5 hours in seconds

void initGPS()
{
    gpsSerial.begin(9600, SERIAL_8N1, RXPin, TXPin); // Start GPS communication
}

double updateGPSAndCalculateDistance()
{
    // Read data from GPS module
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());

        // If new location data is available and has valid GPS fix
        if (gps.location.isUpdated())
        {
            double currentLatitude = gps.location.lat();
            double currentLongitude = gps.location.lng();
            GPS last = readLastCoordinates();

            if (last.latitude != 0.0 && last.longitude != 0.0)
            {
                // Calculate distance from the last known position
                lastDistance = gps.distanceBetween(last.latitude, last.longitude, currentLatitude, currentLongitude);
            }

            // If the distance is less than 1 km, disregard it
            if (lastDistance < distanceOffset)
            {
                writeGPSToCSV(currentLatitude, currentLongitude, 0.0);
                return 0.0; // No significant movement
            }

            // Write to CSV or perform other actions
            writeGPSToCSV(currentLatitude, currentLongitude, lastDistance / 1000); // Convert to km if needed

            return lastDistance / 1000; // Return distance in kilometers
        }
    }

    return 0.0; // Return 0 if no new location is updated
}

String getCurrentTimeStamp()
{
    // Read GPS data
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.time.isValid())
    {
        // Get current UTC time from GPS
        unsigned long utcTime = gps.time.hour() * 3600 + gps.time.minute() * 60 + gps.time.second();

        // Apply the timezone offset
        unsigned long localTime = utcTime + TIMEZONE_OFFSET;

        // Calculate hours, minutes, and seconds for local time
        int localHour = (localTime / 3600) % 24;
        int localMinute = (localTime % 3600) / 60;
        int localSecond = localTime % 60;

        // Handle date change if timezone adjustment changes the day
        int day = gps.date.day();
        int month = gps.date.month();
        int year = gps.date.year();

        if (localHour < 0)
        {
            localHour += 24;
            day -= 1; // Go back a day if local time goes negative
        }
        else if (localHour >= 24)
        {
            localHour -= 24;
            day += 1; // Move to the next day if the time crosses midnight
        }

        // Format the time and date as hh:mm - day-month-year
        char timeStamp[30];
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d - %02d-%02d-%04d", localHour, localMinute, day, month, year);

        return String(timeStamp);
    }
    else
    {
        Serial.println("GPS date or time not valid yet.");
        return "";
    }
}

GPSData getGPSData()
{
    GPSData gpsData;
    gpsData.sat = 0;
    gpsData.fix = 0;
    gpsData.speed = 0;
    while (gpsSerial.available() > 0)
    {
        gps.encode(gpsSerial.read());
    }

    if (gps.satellites.isValid())
    {
        gpsData.sat = gps.satellites.value();
        gpsData.fix = gps.location.isValid() ? (gps.satellites.value() >= 4 ? 3 : 2) : 0;
        gpsData.speed = gps.speed.kmph();
        return gpsData;
    }
    else
    {
        return gpsData;
    }
}