#include <Arduino.h>
#include "relay.h"
#include "elm.h"
extern bool carUnlocked;
const int unLockRelayPin = 14;
const int lockRelayPin = 12;
extern int lockStatus;

void initRelays()
{
    pinMode(lockRelayPin, OUTPUT);
    digitalWrite(lockRelayPin, HIGH); // Turn relay off initially
    pinMode(unLockRelayPin, OUTPUT);
    digitalWrite(unLockRelayPin, HIGH); // Turn relay off initially
}

void lockCar()
{
    float speed = myELM327.kph();

    if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }

    if (speed >= 20)
    {
        Serial.print("Locking Car");
        digitalWrite(lockRelayPin, LOW); // Turn relay on
        delay(500);
        digitalWrite(lockRelayPin, HIGH); // Turn relay off
        carUnlocked = false;
        lockStatus = 1;
    }
}
void unLockCar()
{
    if (carUnlocked)
        return;

    Serial.print("Un-Locking Car");
    digitalWrite(unLockRelayPin, LOW); // Turn relay on
    delay(500);
    digitalWrite(unLockRelayPin, HIGH); // Turn relay off
    carUnlocked = true;
    lockStatus = 0;
}
