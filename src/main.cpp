#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "temperature.h"
#include "display.h"
#include "sd.h"
#include "altitude.h"
#include "buzzer.h"
#include "remote.h"
#include "elm.h"
#include "gps.h"
#include "average.h"
#include "fuel_level.h"
#include "relay.h"

bool firstAltitudeReading = true;
bool selected = false;
bool averageUpdated = false;
unsigned long lastInteractionMillis = 0; // Timer for user interaction
unsigned long previousTempMillis = 0;    // Stores last time temp readings were updated
unsigned long previousCoolantMillis = 0; // Stores last time coolant readings were updated
unsigned long previousAvgMillis = 0;     // Stores last time avg readings were updated
unsigned long previousLogMillis = 0;     // Stores last time data was logged
const long logInterval = 60000;          // Interval for logging data (1 minute)
const long tempInterval = 30000;         // Interval for displaying temperature readings (5 minutes)
const long gpsInterval = 10000;          // Interval for displaying temperature readings (10 seconds)
const long tempSelection = 5000;         // Interval for displaying temperature
int currentMenu = 0;
static bool isStarted = true;
static bool carStarted = true;
bool isMenuActive = true;
bool menuDrawn = false;  // Flag to check if the menu has already been drawn
int unLockRelayPin = 14; // Replace with the GPIO pin you're using for the relay
int lockRelayPin = 12;   // Replace with the GPIO pin you're using for the relay
int hallSensorPin = 35;
int toggleButtonPin = 26;
int lockStatusButtonPin = 27;
bool actionTriggered = false; // Flag to track if toggle has been triggered
bool carUnlocked = true;
bool lastButtonState = HIGH; // Assume the button starts unpressed (HIGH)
bool currentButtonState = HIGH;
int lockStatus = 0;

void setup()
{
    Serial.begin(9600);
    delay(500);
    // initialize Sensors & Displays
    initSDCard(true);
    initializeBluetooth();
    initAltitude();
    initDisplays();
    initTempSensors();
    initIRSensor();
    initFuelLevelSensor();
    initGPS();
    initBuzzer();
    turnBuzzerOn(true);
    initRelays();
    pinMode(hallSensorPin, INPUT);
    pinMode(toggleButtonPin, INPUT_PULLUP);
    pinMode(lockStatusButtonPin, INPUT_PULLUP);
    Serial.println("Initialization Completed.....");
    delay(150);
}

void checkCentralLocking()
{
    int currentButtonState = digitalRead(lockStatusButtonPin);

    if (currentButtonState != lastButtonState)
    {
        if (currentButtonState == LOW)
        {
            Serial.println("Car Is Unlocked");
            carUnlocked = true;
            lockStatus = 0;
        }
        else
        {
            Serial.println("Car Is Locked");
            carUnlocked = false;
            lockStatus = 1;
        }

        lastButtonState = currentButtonState;
    }

    if (carUnlocked && lockStatus == 0)
    {
        lockCar();
    }

    if ((digitalRead(hallSensorPin) == LOW || digitalRead(toggleButtonPin) == LOW) && !actionTriggered) // Assuming LOW means detected
    {
        unLockCar();
        actionTriggered = true;
    }
    else if (digitalRead(hallSensorPin) == HIGH && digitalRead(toggleButtonPin) == HIGH)
    {
        actionTriggered = false;
    }
}

void loop()
{
    decodeIR();
    unsigned long currentMillis = millis(); // Get the current time
    unsigned long currentAvgMillis = millis();

    checkCentralLocking();

    switch (currentMenu)
    {
    case 0:
        isStarted = true;
        if (!menuDrawn)
        {
            drawMenu(0);
            delay(100);
            Average lastAvg = readLastAverageData();
            drawAvgScreen(lastAvg.average, lastAvg.distanceTraveled, lastAvg.fuelRemaining, lastAvg.dte);
            menuDrawn = true;
        }
        break;
    case 1:
        if (isStarted)
        {
            GPSData gpsData = getGPSData();
            drawGPSScreen(false, gpsData.sat, gpsData.speed, gpsData.fix);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= gpsInterval)
        {
            if (averageUpdated)
            {
                GPSData gpsData = getGPSData();
                drawGPSScreen(false, gpsData.sat, gpsData.speed, gpsData.fix);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                GPSData gpsData = getGPSData();
                drawGPSScreen(true, gpsData.sat, gpsData.speed, gpsData.fix);
            }
        }
        break;
    case 2:
        if (isStarted)
        {
            float coolantTemp = readCoolantTemp();
            // float coolantTemp = 10.0;
            delay(200);
            Temperatures temp = getTemperatures();
            drawTemperaturesScreen(temp, false, coolantTemp);

            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            Temperatures temp = getTemperatures();
            float coolantTemp = readCoolantTemp();
            // float coolantTemp = 10.0;
            delay(200);
            if (averageUpdated)
            {
                drawTemperaturesScreen(temp, false, coolantTemp);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                Temperatures temp = getTemperatures();
                drawTemperaturesScreen(temp, true, coolantTemp);
            }
        }
        break;
    case 3:
        if (isStarted)
        {
            Altitude altitude = getAltitude();
            Temperatures temp = getTemperatures();
            drawAltitudeScreen(altitude, temp, false);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= tempInterval)
        {
            Temperatures temp = getTemperatures();
            if (averageUpdated)
            {
                Altitude altitude = getAltitude(); // Get the second reading
                delay(150);
                drawAltitudeScreen(altitude, temp, false);
                averageUpdated = false;
            }
            else
            {
                previousTempMillis = currentMillis;
                Altitude altitude = getAltitude();
                drawAltitudeScreen(altitude, temp, true);
            }
        }
        break;
    case 4:
        if (isStarted)
        {
            EngineInfo engine = getEngineInfo();
            float fuel = getFuelAvailable();
            float vin = readVoltage();
            drawEngineScreen(engine.rpm, engine.coolantTemp, vin, fuel, false);
            isStarted = false;
        }
        else if (currentMillis - previousTempMillis >= gpsInterval)
        {
            EngineInfo engine = getEngineInfo();
            float fuel = getFuelAvailable();
            if (averageUpdated)
            {

                drawEngineScreen(engine.rpm, engine.coolantTemp, engine.speed, fuel, false);
                averageUpdated = false;
            }
            else
            {
                drawEngineScreen(engine.rpm, engine.coolantTemp, engine.speed, fuel, true);
                previousTempMillis = currentMillis;
            }
        }
        break;
    case 5:
        if (isStarted)
        {
            if (averageUpdated)
            {
                drawServiceScreen();
                averageUpdated = false;
            }
            else
            {
                drawServiceScreen();
            }
            isStarted = false;
        }
        break;

    default:
        if (!menuDrawn)
        {
            drawMenu(0);
            menuDrawn = true;
        }
        break;
    }

    if (!selected && currentMillis >= tempSelection)
    {
        currentMenu = 2;
        selected = true;
    }

    if (carStarted)
    {
        float fuelFlow = readFuelFlow();
        double distance = updateGPSAndCalculateDistance();
        logAverageData(fuelFlow, distance);
        carStarted = false;
    }
    else if (currentMillis - previousLogMillis >= logInterval)
    {
        previousLogMillis = currentMillis;
        if (isArrayFull())
        {
            Average avg = calculateAverageFuelConsumption();
            drawAvgScreen(avg.average, avg.distanceTraveled, avg.fuelRemaining, avg.dte);
            averageUpdated = true;
            resetArray();
        }

        else
        {
            float fuelFlow = readFuelFlow();
            double distance = updateGPSAndCalculateDistance();
            logAverageData(fuelFlow, distance);
        }
    }

    if (currentMillis - previousCoolantMillis >= gpsInterval)
    {
        previousCoolantMillis = currentMillis;
        float coolantTemp = readCoolantTemp();
        // float coolantTemp = 10.0;
        delay(200);
        if (coolantTemp >= 100)
        {
            turnBuzzerOn(false);
        }
    }
}