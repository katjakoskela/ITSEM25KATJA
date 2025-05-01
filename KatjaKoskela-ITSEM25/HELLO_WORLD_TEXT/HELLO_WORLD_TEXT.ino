#include <TFT_eSPI.h> // Include the graphics library (TFT_eSPI)
#include <SPI.h>      // Include SPI library

TFT_eSPI tft = TFT_eSPI(); // Create an instance of TFT_eSPI

void setup() {
  tft.init();                // Initialize the display
  tft.setRotation(1);        // Set display rotation (adjust if needed)
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color to white and background to black
  tft.setTextSize(2);        // Set text size (1 = small, 2 = medium, etc.)
  
  tft.setCursor(10, 50);     // Set starting position for the text
  tft.print("HELLO WORLD");  // Print "HELLO WORLD" to the display
}

void loop() {
  // Nothing to do here
}

