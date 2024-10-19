#include <Arduino.h>
#include "buzzer.h"


void initBuzzer()
{
  pinMode(13, OUTPUT);
}


void turnBuzzerOn(bool turn)
{
  digitalWrite(13, HIGH); 
  if (turn)
  {
    delay(150);                   
    digitalWrite(13, LOW); 
  }
}
