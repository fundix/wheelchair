#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h> // https://github.com/adafruit/Adafruit_SH110X
#include "driver/gpio.h"
#include "driver/twai.h"
#include "config.hpp"
#include "Joystick.hpp"
#include "WheelController.hpp"
// Initialize display with correct pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create sprite for top bar (128x16)
static const uint16_t TOP_BAR_HEIGHT = 16;
GFXcanvas1 topBar(SCREEN_WIDTH, TOP_BAR_HEIGHT);

// Create sprites for main content areas (128x48)
static const uint16_t MAIN_HEIGHT = 48; // 64 - 16 = 48
GFXcanvas1 contentArea1(SCREEN_WIDTH, MAIN_HEIGHT);
GFXcanvas1 contentArea2(SCREEN_WIDTH, MAIN_HEIGHT);

// TWAI variables
// twai_message_t message;
// uint32_t can_id = 0;

Joystick *joystick = nullptr;
void analog_task(void *parameter);
void display_loop();

void screen1()
{
  // contentArea1.fillScreen(SH110X_BLACK);
  // contentArea1.setTextSize(1);
  // contentArea1.setTextColor(SH110X_WHITE);
  // contentArea1.setCursor(5, 2);
  // contentArea1.print("CAN MsgID:");
  // contentArea1.setCursor(5, 12);
  // contentArea1.printf("0x%08X", can_id); // HEX výpis

  float x = joystick->getX();
  float y = joystick->getY();

  int centerX = 105;
  int centerY = 15;
  int endX = centerX + static_cast<int>(x * 20);
  int endY = centerY - static_cast<int>(y * 20);

  contentArea1.drawCircle(centerX, centerY, 2, SH110X_WHITE);
  contentArea1.drawLine(centerX, centerY, endX, endY, SH110X_WHITE);
  contentArea1.drawCircle(centerX, centerY, 12, SH110X_WHITE);

  MotorCommand leftMotor, rightMotor;
  joystick->computeMotorCommands(leftMotor, rightMotor);

  contentArea1.setCursor(5, 22);
  contentArea1.print("L:");
  contentArea1.print(leftMotor.reverse ? "R " : "F ");
  contentArea1.print(leftMotor.speed);

  contentArea1.setCursor(50, 22);
  contentArea1.print("R:");
  contentArea1.print(rightMotor.reverse ? "R " : "F ");
  contentArea1.print(rightMotor.speed);

  display.drawBitmap(0, 16, contentArea1.getBuffer(), SCREEN_WIDTH, 48, SH110X_WHITE);

  ESP_LOGI("Joystick", "X: %.2f, Y: %.2f | Levý motor: %s %d | Pravý motor: %s %d",
           joystick->getX(), joystick->getY(),
           leftMotor.reverse ? "REV" : "FWD", leftMotor.speed,
           rightMotor.reverse ? "REV" : "FWD", rightMotor.speed);
}

void screen2()
{
  contentArea2.fillScreen(SH110X_BLACK);
  contentArea2.setTextSize(2);
  contentArea2.setTextColor(SH110X_WHITE);
  contentArea2.setCursor(10, 10);
  contentArea2.print("Area 2");
  display.drawBitmap(0, 16, contentArea2.getBuffer(), SCREEN_WIDTH, 48, SH110X_WHITE);
}

void setup()
{
  Serial.begin();
  Serial.setDebugOutput(true);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  // Initialize I2C with custom pins
  Wire.end();
  Wire.begin(SDA_PIN, SCL_PIN);
  // Wire.setClock(1000000UL); // Set I2C clock to 400kHz

  joystick = new Joystick(JOYSTICK_X, JOYSTICK_Y, 100);

  // Initialize display
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  xTaskCreatePinnedToCore(
      [](void *parameter)
      {
        while (true)
        {
          display_loop();
          vTaskDelay(pdMS_TO_TICKS(25));
        }
      },
      "DisplayTask",
      2048, // Stack size
      NULL, // Task parameters
      1,    // Priority (lower than default loop priority of 1)
      NULL, // Task handle
      1     // Core ID (same as Arduino loop)
  );

  xTaskCreatePinnedToCore(
      analog_task,
      "AnalogTask",
      4096, // stack size
      NULL,
      1, // priority
      NULL,
      0 // core
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
  display.drawBitmap(0, 0, topBar.getBuffer(), SCREEN_WIDTH, 16, SH110X_WHITE);
}

void draw_main_screen()
{
  // Static variable to keep track of current screen
  static uint8_t current_screen = 0;

  // Switch between different screens
  switch (current_screen)
  {
  case 0:
    screen1();
    break;
  case 1:
    screen2();
    break;
  // Add more cases here for additional screens
  default:
    current_screen = 0; // Reset to first screen if invalid
    screen1();
    break;
  }

  static uint32_t lastButtonPress = 0;
  const uint32_t debounceTime = 250; // 250ms debounce

  // Check button with debouncing
  uint32_t currentTime = esp_timer_get_time() / 1000; // Convert to milliseconds
  if (digitalRead(BUTTON_PIN_OK) == LOW)
  { // Assuming active LOW button
    if (currentTime - lastButtonPress > debounceTime)
    {
      current_screen = (current_screen + 1) % 2; // Toggle between 2 screens
      lastButtonPress = currentTime;
    }
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
  const uint8_t outerRadius = 6;
  const uint8_t innerRadius = 2;

  // Draw outer circle
  spinner.drawCircle(centerX, centerY, outerRadius + 1, SH110X_WHITE);
  spinner.drawCircle(centerX, centerY, outerRadius, SH110X_WHITE);
  spinner.fillCircle(centerX, centerY, innerRadius, SH110X_WHITE);
  // spinner.fillc

  // Draw spokes
  for (int i = 0; i < 6; i++)
  {
    float angle = (spinnerPos * PI / 12.0) + (i * PI / 3.0);
    int16_t endX = centerX + (outerRadius * cos(angle));
    int16_t endY = centerY + (outerRadius * sin(angle));
    spinner.drawLine(centerX, centerY, endX, endY, SH110X_WHITE);
  }

  // Update spinner position for next frame (slower rotation)
  spinnerPos = (spinnerPos - 3) % 24;
  // Draw spinner at bottom right
  static int16_t spinnerX = -16;                   // Start from off-screen left
  spinnerX = (spinnerX + 3) % (SCREEN_WIDTH + 16); // Include full width of spinner
  if (spinnerX == SCREEN_WIDTH)
    spinnerX = -16; // Reset when completely off-screen
  display.drawBitmap(spinnerX, SCREEN_HEIGHT - 17,
                     spinner.getBuffer(), 16, 16, SH110X_WHITE);
  // ============================ spinner ===================================

  // Update display
  display.display();

  // delay(10);
}

void loop()
{
  // put your main code here, to run repeatedly:

  // if (twai_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK)
  // {
  //   can_id = message.identifier; // Uložíme ID pro výpis
  // }
}

void analog_task(void *parameter)
{
  // Příklad: připojení joysticku na ADC kanály ADC1_CHANNEL_6 a ADC1_CHANNEL_7
  extern Joystick *joystick;
  joystick->calibrate();

  while (1)
  {
    joystick->update();

    vTaskDelay(pdMS_TO_TICKS(25)); // Delay for 25 ms
  }
}