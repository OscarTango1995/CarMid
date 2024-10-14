#include <Arduino.h>
#include"temperature.h"
#include"display.h"
#include"logger.h"
#include"altitude.h"
#include"buzzer.h"

unsigned long previousTempMillis = 0; // Stores last time temp readings were updated
unsigned long previousAvgMillis = 0; // Stores last time avg readings were updated
const long tempInterval = 30000; // Interval for displaying temperature readings (30 seconds)
const long tempSelection = 10000; // Interval for displaying temperature
const long avgInterval = 900000; // Interval for displaying the average screen (30 seconds)
int currentMenu = 0; 
static bool isStarted=true;
bool isMenuActive = true;
bool menuDrawn = false;               // Flag to check if the menu has already been drawn


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

// initialize Sensors & Displays
    initAltitude(); 
    initDisplays();
    initTempSensors(); 
    //initLogging();
    initBuzzer();
    turnBuzzerOn(true);
 Serial.print("init done");
    delay(1000);
}

void loop() {
   unsigned long currentMillis = millis(); // Get the current time
    if(currentMillis - previousTempMillis >= tempSelection) {
        currentMenu=3;
    }
   
    switch (currentMenu)
    {
        case 0:
            if (!menuDrawn) {
                   drawMenu();  
                   drawAvgScreen();       // Call drawMenu only once when currentMenu is 0
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
                Temperatures temp = getTemperatures(); 
                drawTemperaturesScreen(temp);
                drawAvgScreen();
            }
        break;
        case 3:
        if (isStarted){
                Altitude altitude = getAltitude(); 
                Temperatures temp = getTemperatures();
                drawAltitudeScreen(altitude,temp);
                isStarted=false;
            }
            else if(currentMillis - previousTempMillis >= tempInterval) {
                previousTempMillis = currentMillis;
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
                    drawMenu();         
                    menuDrawn = true;
                }
    break;
    }
   
    if (currentMillis - previousAvgMillis >= avgInterval) {
        previousAvgMillis = currentMillis; 
        // getAvg();yet to be implemented
        drawAvgScreen();
    }

}