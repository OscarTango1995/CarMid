#include <Arduino.h>
#include "buzzer.h"

const int buzzerPin = 13;

// Initialize the buzzer
void initBuzzer()
{
  pinMode(buzzerPin, OUTPUT); // Set pin as output
}

// Turn the buzzer on for 1 second
void turnBuzzerOn(bool turn)
{
  digitalWrite(buzzerPin, HIGH);
  delay(150); // Wait for 1 second
  if (turn)
  {
    digitalWrite(buzzerPin, LOW);
    delay(1000); // Wait for 1 second
  }
}
