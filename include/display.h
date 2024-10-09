#ifndef DISPLAY_H
#define DISPLAY_H
#include"temperature.h"

// Function declarations for initializing and updating displays
void initDisplays();
// Function to display the main menu
void drawMenu();

// Function to display temperatures on the OLED
void drawTemperaturesScreen(Temperatures temp);

#endif // DISPLAY_H
