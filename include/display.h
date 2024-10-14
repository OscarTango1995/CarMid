#ifndef DISPLAY_H
#define DISPLAY_H
#include"temperature.h"
#include"altitude.h"

// Function declarations for initializing and updating displays
void initDisplays();
// Function to display the main menu
void drawMenu();

// Function to display temperatures on the OLED
void drawTemperaturesScreen(Temperatures temp);
void drawAltitudeScreen(Altitude altitude,Temperatures temp);
void drawAvgScreen();

#endif // DISPLAY_H
