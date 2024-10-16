#include <Arduino.h>
#include "IRremote.h"
#include "remote.h"
#include"display.h"

// Pin for the IR receiver
const int RECV_PIN = 16; 

// Hex values for remote buttons
#define BUTTON_UP 0xFE01EF00
#define BUTTON_DOWN 0xF609EF00
#define BUTTON_BACK 0xFB04EF00
#define BUTTON_SELECT 0xFA05EF00
#define BUTTON_RESET 0xFC03EF00


#define ZERO 0xF50AEF00
#define ONE 0xF40BEF00
// #define TWO 0xFB04EF00
// #define THREE 0xFA05EF00
// #define FOUR 0xFE01EF00
// #define FIVE 0xF609EF00
// #define SIX 0xFB04EF00
// #define SEVEN 0xFA05EF00
// #define EIGHT 0xFE01EF00
// #define NINE 0xF609EF00


// Variables for menu control
extern int currentMenu;
extern bool menuDrawn;
extern bool isStarted;
int selectedItem = 0;  // Initially, the first menu item is selected
int menuItemCount = 5;  // Total number of menu items

// Create an IR receiver object
decode_results results;

// Initialize the buzzer pin
void initIRSensor() {
    IrReceiver.begin(RECV_PIN,ENABLE_LED_FEEDBACK);  // Start the receiver
}

void handleMenuNavigation(unsigned long irCode) {
    switch (irCode) {
        case BUTTON_UP: // Up button
            selectedItem--;
            if (selectedItem < 0) {
                selectedItem = menuItemCount - 1;  // Wrap around to the last item
            }
            drawMenu(selectedItem);  // Redraw the menu with the updated selection
            break;
        case BUTTON_DOWN: // Down button
            selectedItem++;
            if (selectedItem >= menuItemCount) {
                selectedItem = 0;  // Wrap around to the first item
            }
            drawMenu(selectedItem);  // Redraw the menu with the updated selection
            break;
        case BUTTON_SELECT: // Select button
            // Handle the selected item action
            handleMenuAction(selectedItem);
            break;
        case BUTTON_BACK: // Back button
            currentMenu = 0; // Set to main menu
            selectedItem = 0; // Reset selected item to the first item
            drawMenu(selectedItem); // Redraw the main menu
            break;    
    }
}

void handleMenuAction(int selectedItem) {
    switch (selectedItem) {
        case 0:
            // Action for "GPS" menu item
            currentMenu = 1;
            Serial.println("GPS selected");
            break;
        case 1:
            // Action for "Temperatures" menu item
            currentMenu = 2;
            Serial.println("Temperatures selected");
            break;
        case 2:
            // Action for "Altitude" menu item
            currentMenu = 3;
            Serial.println("Altitude selected");
            break;
        case 3:
            // Action for "Average" menu item
            currentMenu = 4;
            Serial.println("Average selected");
            break;
        case 4:
            // Action for "Service" menu item
            currentMenu = 5;
            Serial.println("Service selected");
            break;
        // Handle other cases if necessary
    }
}

void decodeIR(){
     if (IrReceiver.decode()) {
        unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;
        handleMenuNavigation(irCode); // Call to handle menu selection
        IrReceiver.resume(); // Enable receiving of the next value
    }
}
