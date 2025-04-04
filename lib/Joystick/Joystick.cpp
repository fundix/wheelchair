#include "Joystick.hpp"

// Konstruktor přijímá ADC kanály pro X a Y a volitelnou hodnotu dead zone (výchozí je 1/4 rozsahu ADC)
Joystick::Joystick(adc1_channel_t channelX, adc1_channel_t channelY, int deadZone)
    : _channelX(channelX), _channelY(channelY), _deadZone(deadZone),
      _zeroX(0), _zeroY(0), _rawX(0), _rawY(0),
      _normX(0.0f), _normY(0.0f)
{
    // Inicializace ADC – konfigurace šířky převodníku a útlumu pro dané kanály
    adc1_config_width(ADC_WIDTH_BIT_12);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 0, NULL);
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
}

// Aktualizace – načte aktuální hodnoty z ADC, odečte nulovou pozici a normalizuje rozdíl
void Joystick::update()
{
    _rawX = adc1_get_raw(_channelX);
    _rawY = adc1_get_raw(_channelY);

    int diffX = _rawX - _zeroX;
    int diffY = _rawY - _zeroY;

    // Pokud je odchylka menší než dead zone, považujeme ji za nulu
    if (abs(diffX) < _deadZone)
        diffX = 0;
    if (abs(diffY) < _deadZone)
        diffY = 0;

    // Normalizace – předpokládáme maximální odchylku jako polovinu rozsahu ADC
    float maxDeviation = ADC_MAX / 2.0f;
    _normX = fmaxf(fminf((float)diffX / maxDeviation, 1.0f), -1.0f);
    _normY = fmaxf(fminf((float)diffY / maxDeviation, 1.0f), -1.0f);
}

// Přístupové metody k normalizovaným hodnotám
float Joystick::getX() const { return _normX; }
float Joystick::getY() const { return _normY; }

// Výpočet příkazů pro motory podle principu arcade drive:
// Kombinací hodnot pro vpřed/zad (_normY) a zatáčení (_normX)
// Například při úplném odchýlení doleva (_normX = -1, _normY = 0) se levý motor roztočí vzad a pravý vpřed.
void Joystick::computeMotorCommands(MotorCommand &leftMotor, MotorCommand &rightMotor)
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
