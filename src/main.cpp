#include <Arduino.h>
#include"temperature.h"
#include"display.h"
#include"logger.h"
#include"altitude.h"
#include"buzzer.h"

unsigned long previousMillis = 0; // Stores last time readings were updated
const long tempInterval = 30000; // Interval for displaying temperature readings
const long tempSelection = 10000; // Interval for displaying temperature
int currentMenu = 0; // Hard-coded to select "Temperatures" as default
static bool isStarted=true;
bool isMenuActive = true;
bool menuDrawn = false;               // Flag to check if the menu has already been drawn


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

// initialize Sensors & Displays
  initDisplays();
  initTempSensors(); 
//   initAltitude(); 
//initLogging();
    initBuzzer();
    turnBuzzerOn(true);
 
    delay(1000);
}

void loop() {
   unsigned long currentMillis = millis(); // Get the current time

    if(currentMillis - previousMillis >= tempSelection) {
        currentMenu=2;
    }

    switch (currentMenu)
    {
        case 0:
            if (!menuDrawn) {
                   drawMenu();         // Call drawMenu only once when currentMenu is 0
                   menuDrawn = true;   // Set flag to prevent repeated calls
               }
        break;
        case 1:
        /* code */
        break;
        case 2:
        if (isStarted){
            Temperatures temp = getTemperatures(); 
            drawTemperaturesScreen(temp);
            isStarted=false;
            }
            else if(currentMillis - previousMillis >= tempInterval) {
                previousMillis = currentMillis;
                Temperatures temp = getTemperatures(); // Read temperature data
                drawTemperaturesScreen(temp);// Update OLED with new readings
            }
        break;
        case 3:
        if (isStarted){
                Altitude altitude = getAltitude(); 
                Temperatures temp = getTemperatures();
                drawAltitudeScreen(altitude,temp);
                isStarted=false;
            }
            else if(currentMillis - previousMillis >= tempInterval) {
                previousMillis = currentMillis;
                 Altitude altitude = getAltitude(); 
                Temperatures temp = getTemperatures();
                drawAltitudeScreen(altitude,temp);
            }
        break;
        case 4:
        /* code */
        break;
        case 5:
        /* code */
        break;

    default:
        if(!menuDrawn) {
                    drawMenu();         // Call drawMenu only once when currentMenu is 0
                    menuDrawn = true;   // Set flag to prevent repeated calls
                }
    break;
    }
   
}