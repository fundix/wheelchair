#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h> // https://github.com/adafruit/Adafruit_SH110X
#include "driver/gpio.h"
#include "driver/twai.h"
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

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL); // TX=5, RX=4
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK &&
      twai_start() == ESP_OK)
  {
    Serial.println("TWAI driver started");
  }
  else
  {
    Serial.println("Failed to start TWAI driver");
  }

  log_d("setup done");
  Serial.printf("setup done\n");
}

void loop()
{
  // put your main code here, to run repeatedly:

  static bool showArea1 = true;
  static unsigned long lastSwitch = 0;

  twai_message_t message;
  uint32_t can_id = 0;
  if (twai_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK)
  {
    can_id = message.identifier; // Uložíme ID pro výpis
  }

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
    // contentArea1.fillScreen(SH110X_BLACK);
    // contentArea1.setTextSize(2);
    // contentArea1.setTextColor(SH110X_WHITE);
    // contentArea1.setCursor(10, 10);
    // contentArea1.print("Area 1");
    // display.drawBitmap(0, 16, contentArea1.getBuffer(), 128, 48, SH110X_WHITE);

    contentArea1.fillScreen(SH110X_BLACK);
    contentArea1.setTextSize(1);
    contentArea1.setTextColor(SH110X_WHITE);
    contentArea1.setCursor(2, 10);
    contentArea1.print("CAN MsgID:");
    contentArea1.setCursor(2, 30);
    contentArea1.printf("0x%08X", can_id); // HEX výpis
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