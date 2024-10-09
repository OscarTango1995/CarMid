#include <Arduino.h>
#include"temperature.h"
#include"display.h"
#include"logger.h"

unsigned long previousTempMillis = 0; // Stores last time readings were updated
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
  // initLogging();
  
  delay(1000);
}

void loop() {
   unsigned long currentMillis = millis(); // Get the current time

    if(currentMillis - previousTempMillis >= tempSelection) {
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
            else if(currentMillis - previousTempMillis >= tempInterval) {
                previousTempMillis = currentMillis;
                Temperatures temp = getTemperatures(); // Read temperature data
                drawTemperaturesScreen(temp);// Update OLED with new readings
            }
        break;

    default:
        if(!menuDrawn) {
                    drawMenu();         // Call drawMenu only once when currentMenu is 0
                    menuDrawn = true;   // Set flag to prevent repeated calls
                }
    break;
    }
   
}