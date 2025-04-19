#include "Joystick.hpp"
#include "esp_adc_cal.h"

static const char *TAG = "Joystick";

// Konstruktor přijímá ADC kanály pro X a Y a volitelnou hodnotu dead zone (výchozí je 1/4 rozsahu ADC)
Joystick::Joystick(adc1_channel_t channelX, adc1_channel_t channelY, int deadZone)
    : _channelX(channelX),
      _channelY(channelY), _deadZone(deadZone),
      _zeroX(0), _zeroY(0), _rawX(0), _rawY(0),
      _normX(0.0f), _normY(0.0f), _minX(0), _maxX(0), _minY(0), _maxY(0)
{
    // Inicializace ADC – konfigurace šířky převodníku a útlumu pro dané kanály
    adc1_config_width(ADC_WIDTH_BIT_12);
    static esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    adc1_config_channel_atten(_channelX, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(_channelY, ADC_ATTEN_DB_12);
}

// Kalibrace – průměruje několik vzorků pro určení nulové (středové) pozice joysticku
void Joystick::calibrate(int samples)
{
    long sumX = 0, sumY = 0;
    for (int i = 0; i < samples; i++)
    {
        sumX += adc1_get_raw(_channelX);
        sumY += adc1_get_raw(_channelY);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    _zeroX = sumX / samples;
    _zeroY = sumY / samples;
    ESP_LOGI(TAG, "Center calibration X: %d, Y: %d", _zeroX, _zeroY);
}

// Aktualizace – načte aktuální hodnoty z ADC, odečte nulovou pozici a normalizuje rozdíl
void Joystick::update()
{
    if (_maxX == _minX || _maxY == _minY)
    {
        ESP_LOGW(TAG, "Joystick min/max values not calibrated properly.");
        _normX = 0.0f;
        _normY = 0.0f;
        return;
    }

    _rawX = adc1_get_raw(_channelX);
    _rawY = adc1_get_raw(_channelY);

    _normX = 0.0f;
    _normY = 0.0f;

    if (_rawX < _zeroX && _zeroX != _minX)
        _normX = -fminf((float)(_zeroX - _rawX) / fabsf((float)(_zeroX - _minX)), 1.0f);
    else if (_rawX > _zeroX && _maxX != _zeroX)
        _normX = fminf((float)(_rawX - _zeroX) / fabsf((float)(_maxX - _zeroX)), 1.0f);

    if (_rawY < _zeroY && _zeroY != _minY)
        _normY = -fminf((float)(_zeroY - _rawY) / fabsf((float)(_zeroY - _minY)), 1.0f);
    else if (_rawY > _zeroY && _maxY != _zeroY)
        _normY = fminf((float)(_rawY - _zeroY) / fabsf((float)(_maxY - _zeroY)), 1.0f);

    // Dead zone filter for small noise
    if (fabsf(_normX) < 0.05f)
        _normX = 0.0f;
    if (fabsf(_normY) < 0.05f)
        _normY = 0.0f;
}

float Joystick::getX() const
{
    return _normX;
}

float Joystick::getY() const
{
    return _normY;
}

void Joystick::computeMotorCommands(WheelCommand &leftMotor, WheelCommand &rightMotor)
{
    float throttle = _normY;
    float turn = _normX;

    float leftOutput = throttle + turn;
    float rightOutput = throttle - turn;

    // Omezení výsledků do intervalu -1 až 1
    leftOutput = fmaxf(fminf(leftOutput, 1.0f), -1.0f);
    rightOutput = fmaxf(fminf(rightOutput, 1.0f), -1.0f);

    // Nastavení směru a škálování rychlosti do intervalu 0–255
    leftMotor.reverse = (leftOutput < 0);
    rightMotor.reverse = (rightOutput < 0);
    leftMotor.speed = (uint8_t)(fabs(leftOutput) * 255);
    rightMotor.speed = (uint8_t)(fabs(rightOutput) * 255);
}

void Joystick::calibrateMinMax(int minX, int maxX, int minY, int maxY)
{
    _minX = minX;
    _maxX = maxX;
    _minY = minY;
    _maxY = maxY;
    ESP_LOGI(TAG, "Calibration Min/Max X: [%d, %d], Y: [%d, %d]", _minX, _maxX, _minY, _maxY);
}
