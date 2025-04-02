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

// TWAI variables
twai_message_t message;
uint32_t can_id = 0;

void display_loop();

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

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_io_num, rx_io_num, TWAI_MODE_NORMAL);
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

  xTaskCreatePinnedToCore(
      [](void *parameter)
      {
        while (true)
        {
          display_loop();
          vTaskDelay(pdMS_TO_TICKS(100));
        }
      },
      "DisplayTask",
      2048, // Stack size
      NULL, // Task parameters
      1,    // Priority (lower than default loop priority of 1)
      NULL, // Task handle
      1     // Core ID (same as Arduino loop)
  );

  log_d("setup done");
}

void draw_header()
{
  // Draw top bar
  topBar.fillScreen(SH110X_BLACK);
  topBar.setTextSize(1);
  topBar.setTextColor(SH110X_WHITE);
  topBar.setCursor(5, 4);
  topBar.print("Wheelchair");
  topBar.drawLine(0, TOP_BAR_HEIGHT - 1, SCREEN_WIDTH, TOP_BAR_HEIGHT - 1, SH110X_WHITE);
  display.drawBitmap(0, 0, topBar.getBuffer(), 128, 16, SH110X_WHITE);
}

void draw_main_screen()
{
  // topBar.drawRoundRect(0, 0, SCREEN_WIDTH, TOP_BAR_HEIGHT + 2, 3, SH110X_WHITE);
  static bool showArea1 = true;
  static unsigned long lastSwitch = 0;

  if (millis() - lastSwitch >= 2000)
  {
    showArea1 = !showArea1;
    lastSwitch = millis();
  }

  // Draw content area
  if (showArea1)
  {
    contentArea1.fillScreen(SH110X_BLACK);
    contentArea1.setTextSize(1);
    contentArea1.setTextColor(SH110X_WHITE);
    contentArea1.setCursor(5, 10);
    contentArea1.print("CAN MsgID:");
    contentArea1.setCursor(5, 24);
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
}

void display_loop()
{

  // Clear all display areas
  display.fillScreen(SH110X_BLACK);
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 3, SH110X_WHITE);

  draw_header();
  draw_main_screen();

  // ============================ spinner ===================================
  // Create static canvas for wheel spinner (16x16)
  static GFXcanvas1 spinner(16, 16);
  static uint8_t spinnerPos = 0;

  // Draw wheel spinner in bottom right corner
  spinner.fillScreen(SH110X_BLACK);
  const uint8_t centerX = 8;
  const uint8_t centerY = 8;
  const uint8_t outerRadius = 7;
  const uint8_t innerRadius = 3;

  // Draw outer circle
  spinner.drawCircle(centerX, centerY, outerRadius, SH110X_WHITE);
  spinner.drawCircle(centerX, centerY, innerRadius, SH110X_WHITE);

  // Draw spokes
  for (int i = 0; i < 6; i++)
  {
    float angle = (spinnerPos * PI / 12.0) + (i * PI / 3.0);
    int16_t endX = centerX + (outerRadius * cos(angle));
    int16_t endY = centerY + (outerRadius * sin(angle));
    spinner.drawLine(centerX, centerY, endX, endY, SH110X_WHITE);
  }

  // Update spinner position for next frame (slower rotation)
  spinnerPos = (spinnerPos + 1) % 24;

  // Draw spinner at bottom right
  display.drawBitmap(SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20,
                     spinner.getBuffer(), 16, 16, SH110X_WHITE);
  // ============================ spinner ===================================

  // Update display
  display.display();

  delay(10);
}

void loop()
{
  // put your main code here, to run repeatedly:

  if (twai_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK)
  {
    can_id = message.identifier; // Uložíme ID pro výpis
  }
}