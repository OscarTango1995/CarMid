#include "elm.h"
#include "Arduino.h"

BluetoothSerial SerialBT;
ELM327 myELM327;


void initializeBluetooth()
{
    if (!SerialBT.begin("ArduHUD", true))
    {
        Serial.println("Failed to initialize Bluetooth");
        while (1)
            ;
    }
    SerialBT.setPin("1234");
    connectToOBD();
}

void connectToOBD()
{
    if (!SerialBT.connect("OBDII"))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }
    if (!myELM327.begin(SerialBT, false, 2000))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }
    Serial.println("Connected to ELM327");
}

uint8_t readCoolantTemp()
{
    float coolant = myELM327.engineCoolantTemp();
    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
        return (uint8_t)coolant;
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
    return 0;
}

float readFuelFlow()
{
    float engineLoad = myELM327.engineLoad();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    uint8_t manifoldPressure = myELM327.manifoldPressure();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float throttlePosition = myELM327.throttle();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    float rpm = myELM327.rpm();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }


    float fuelFlow = (engineLoad / 100.0) * (manifoldPressure / 100.0) * (throttlePosition / 100.0) * (rpm / 2.0) * 0.5; //K is 0.5

    return fuelFlow; // Return fuel flow in L/h
}

float calculateAverageFuelConsumption(float fuelFlow)
{
    // Step 1:Calculate speed
    uint8_t speed = myELM327.kph();
    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
    // Step 1: Calculate fuel consumption rate in L/min
    float fuelConsumptionRate = readFuelFlow() / 60.0;

    // Step 2: Convert speed to km/min
    float distanceTraveled = (float)speed / 60.0; // Speed in km/h

    // Step 3: Calculate average fuel consumption in km/L
    if (fuelConsumptionRate < 1)
    {
        fuelConsumptionRate = 1;
    }

    return distanceTraveled / fuelConsumptionRate; // Return average fuel consumption in km/L
}
