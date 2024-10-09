#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include"temperature.h"
#include"altitude.h"


// U8G2 constructor for the OLED (replace 0x3C with the correct address if needed)
U8G2_SH1106_128X64_NONAME_F_SW_I2C oled(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_SW_I2C oled2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);


void drawDisplay(U8G2_SH1106_128X64_NONAME_F_SW_I2C &oled, const char* title, const char* content) {
    oled.clearDisplay();
    oled.clearBuffer();
    
    // Draw the outer frame
    oled.drawFrame(0, 0, 128, 64); 

    // Draw the title bar
    oled.setDrawColor(0);  // Set draw color to black
    oled.drawBox(0, 0, 128, 13);  // Draw a filled box for the title bar
    oled.setDrawColor(1);  // Set draw color to white
    oled.drawFrame(0, 0, 128, 13);  // Draw the border around the title bar
    
    oled.setFont(u8g2_font_6x12_tr);  
    oled.setDrawColor(1);  // Set draw color to white for text

    // Center the title
    oled.setCursor((128 - oled.getStrWidth(title)) / 2, 10);  
    oled.print(title);
    

    // Draw the content with line breaks
 
    oled.setFont(u8g2_font_7x14_tr); // font
    int contentYPosition = 25;  // Start directly below the title bar
    const int leftPadding = 6; // Adjust this value for left padding
    const int lineHeight = 12; // Adjust this value to control vertical spacing

    char* token = strtok((char*)content, "\n");  // Tokenize content by newline
    // Increase the line height for better spacing

    while (token != NULL) {
        oled.setCursor(leftPadding, contentYPosition);  // Set cursor for each line
        oled.print(token);  // Print each line
        contentYPosition += lineHeight;  // Move down for the next line
        token = strtok(NULL, "\n");  // Get the next line
    }


    oled.sendBuffer();
}

// Function to draw a menu with submenus
void drawMenu() {
    oled2.clearDisplay();
    oled2.clearBuffer();
     // Define menu items
    const char* menuItems[] = {
        "1. GPS",
        "2. Temperatures",
        "3. Altitude",
        "4. Average",
        "5. Service"
    };

    int itemCount = sizeof(menuItems) / sizeof(menuItems[0]);

    // Draw the outer frame
    oled2.drawFrame(0, 0, 128, 64);

    // Draw the title bar
    oled2.setDrawColor(0);  // Set draw color to black
    oled2.drawBox(0, 0, 128, 13);  // Draw a filled box for the title bar
    oled2.setDrawColor(1);  // Set draw color to white
    oled2.drawFrame(0, 0, 128, 13);  // Draw the border around the title bar

    oled2.setFont(u8g2_font_6x12_tr);
    oled2.setDrawColor(1);  // Set draw color to white for text

    // Center the title
    oled2.setCursor((128 - oled.getStrWidth("Main Menu")) / 2, 10);
    oled2.print("Main Menu");

   // Draw the menu items
    oled2.setFont(u8g2_font_7x14_tr); // font
    int contentYPosition = 26;  // Start directly below the title bar
    int lineHeight = 12;  // Height of each menu item
    int availableHeight = 64 - contentYPosition - 2; // Total height available for content area minus padding
    int maxItemsInView = availableHeight / lineHeight; // Max number of items that can fit
    const int leftPadding = 6; 

    for (int i = 0; i < itemCount && i < maxItemsInView; i++) {
        oled2.setCursor(leftPadding, contentYPosition + 2 + (i * lineHeight));  // A little padding and space for each item
        oled2.print((const char*)menuItems[i]);  // Print each menu item
    }

    oled2.sendBuffer();
}


void initDisplays() {
    Wire.begin();
    delay(100); 


    oled.setI2CAddress(0x3C << 1);  // Left-shifted I2C address for 7-bit format
    if (!oled.begin()) {
    Serial.println(F("Failed to initialize display"));
    while (1);  // Stay here if the display fails to initialize
    }
    drawDisplay(oled, "Average", "16 KM/L");
    oled.sendBuffer();  // Push the data to the OLED screen
    delay(1000);


    oled2.setI2CAddress(0x3D << 1);  // Left-shifted I2C address for 7-bit format
    if (!oled2.begin()) {
    Serial.println(F("Failed to initialize display"));
    while (1);  // Stay here if the display fails to initialize
    delay(1000);
  }
}

// Function to display temperature data on the OLED
void drawTemperaturesScreen(Temperatures temp) {
    // Create content string for display without floating points
    char content[60]; // Ensure the buffer is large enough for your content
    snprintf(content, sizeof(content), "OAT : %d C\nIAT : %d C\nRH  : %d %% \nENG : %d C", 
             (int)temp.oat, (int)temp.iat, (int)temp.humidity,(int)temp.engine);
    
    // Call drawDisplay to show the temperatures
    drawDisplay(oled2, "TEMPERATURES", content);
        
}

// Function to display altitude data on the OLED
void drawAltitudeScreen(Altitude altitude,Temperatures temp) {
    // Create content string for display without floating points
    char content[60]; // Ensure the buffer is large enough for your content
    snprintf(content, sizeof(content), "ALT : %d M \nHPA : %d\nIAT : %d C \nOAT : %d C", 
             (int)altitude.altitude, (int)altitude.pressure, (int)altitude.temperature,(int)temp.oat);
    
    // Call drawDisplay to show the temperatures
    drawDisplay(oled2, "ALTITUDE", content);
        
}
