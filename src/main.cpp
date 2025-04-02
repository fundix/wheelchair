#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h> // https://github.com/adafruit/Adafruit_SH110X

#include "config.hpp"
// Initialize display with correct pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create sprite for top bar (128x16)
static const uint16_t TOP_BAR_HEIGHT = 16;
GFXcanvas1 topBar(128, TOP_BAR_HEIGHT);

// Create sprites for main content areas (128x48)
static const uint16_t MAIN_HEIGHT = 48; // 64 - 16 = 48
GFXcanvas1 contentArea1(128, MAIN_HEIGHT);
GFXcanvas1 contentArea2(128, MAIN_HEIGHT);

void setup()
{
  Serial.begin();
  Serial.setDebugOutput(true);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  // Initialize I2C with custom pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize display
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  delay(1000);

  log_d("setup done");
  Serial.printf("setup done\n");
}

void loop()
{
  // put your main code here, to run repeatedly:

  static bool showArea1 = true;
  static unsigned long lastSwitch = 0;

  // Clear all display areas
  display.clearDisplay();

  // Draw top bar
  topBar.fillScreen(SH110X_BLACK);
  topBar.setTextSize(1);
  topBar.setTextColor(SH110X_WHITE);
  topBar.setCursor(2, 4);
  topBar.print("Wheelchair");
  display.drawBitmap(0, 0, topBar.getBuffer(), 128, 16, SH110X_WHITE);

  // Switch between content areas every 2 seconds
  if (millis() - lastSwitch >= 2000)
  {
    showArea1 = !showArea1;
    lastSwitch = millis();
  }

  // Draw content area
  if (showArea1)
  {
    contentArea1.fillScreen(SH110X_BLACK);
    contentArea1.setTextSize(2);
    contentArea1.setTextColor(SH110X_WHITE);
    contentArea1.setCursor(10, 10);
    contentArea1.print("Area 1");
    display.drawBitmap(0, 16, contentArea1.getBuffer(), 128, 48, SH110X_WHITE);
  }
  else
  {
    contentArea2.fillScreen(SH110X_BLACK);
    contentArea2.setTextSize(2);
    contentArea2.setTextColor(SH110X_WHITE);
    contentArea2.setCursor(10, 10);
    contentArea2.print("Area 2");
    display.drawBitmap(0, 16, contentArea2.getBuffer(), 128, 48, SH110X_WHITE);
  }

  // Update display
  display.display();
}