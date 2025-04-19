#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ADC_MAX 4095 // 12-bit ADC

// Struktura pro příkaz motoru: směr (reverse) a rychlost (0–255)
struct WheelCommand
{
    bool reverse;
    uint8_t speed;
};

class Joystick
{
public:
    // Constructor
    Joystick(adc1_channel_t channelX, adc1_channel_t channelY, int deadZone = 100);

    // Public methods
    void calibrate(int samples = 20);
    void update();
    float getX() const;
    float getY() const;
    int getRawX() const { return _rawX; }
    int getRawY() const { return _rawY; }
    void computeMotorCommands(WheelCommand &leftMotor, WheelCommand &rightMotor);
    void calibrateMinMax(int minX, int maxX, int minY, int maxY);

private:
    adc1_channel_t _channelX, _channelY;
    int _deadZone;
    int _zeroX, _zeroY;
    int _rawX, _rawY;
    float _normX, _normY;
    int _minX, _maxX;
    int _minY, _maxY;
};

#endif // JOYSTICK_HPP