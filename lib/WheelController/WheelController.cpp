#include "WheelController.hpp"

static const char *TAG = "WhCont";

WheelController::WheelController()
    : joystickX(0), joystickY(0), currentMode(SPEED_HIGH),
      deadzone(0.05), expoFwd(1.3), expoTurn(1.2), turnScaleCoefficient(0.70),
      leftCmd(0), rightCmd(0), measuredLeftSpeed(0), measuredRightSpeed(0),
      smoothingFactor(0.5f), lastLeftCmd(0), lastRightCmd(0), GP8413(i2c_DAC_Address, RESOLUTION_15_BIT)
{
}

void WheelController::begin()
{
    Wire.begin();
    // Inicializace I2C – dle použité knihovny a DAC modulu
    // Inicializace DAC (specificky pro DFR1073) zde
    // např. dac.begin();
    uint8_t retries = 0;
    const uint8_t MAX_RETRIES = 5;

    while (!DAInitialized && retries < MAX_RETRIES)
    {
        int dacInitResult = GP8413.begin();
        ESP_LOGI(TAG, "DAC init returned: %d", dacInitResult);
        if (dacInitResult == 0)
        {
            DAInitialized = true;
        }
        else
        {
            ESP_LOGE(TAG, "Communication with the device failed. Please check if the connection is correct or if the device address is set correctly.");
            delay(100);
            retries++;
        }
    }

    if (!DAInitialized)
    {
        ESP_LOGE(TAG, "Failed to initialize DAC after maximum retries");
    }
    else
    {
        GP8413.setDACOutRange(GP8413.eOutputRange5V);
    }

    // reverse PIN init
    pinMode(MOTOR_LEFT_REVERSE_PIN, OUTPUT);
    digitalWrite(MOTOR_LEFT_REVERSE_PIN, MOTOR_LEFT_INIT_DIR);
    pinMode(MOTOR_RIGHT_REVERSE_PIN, OUTPUT);
    digitalWrite(MOTOR_RIGHT_REVERSE_PIN, MOTOR_RIGHT_INIT_DIR);
}

void WheelController::setJoystickInput(float x, float y)
{
    joystickX = x;
    joystickY = y;
}

void WheelController::setSpeedMode(SpeedMode mode)
{
    currentMode = mode;
}

void WheelController::setTurnScaleCoefficient(float coeff)
{
    turnScaleCoefficient = coeff;
}

void WheelController::setExpoCurveFwd(float expo)
{
    expoFwd = expo;
}

void WheelController::setExpoCurveTurn(float expo)
{
    expoTurn = expo;
}

void WheelController::setDeadzone(float dz)
{
    deadzone = dz;
}

void WheelController::setMeasuredSpeed(float left, float right)
{
    measuredLeftSpeed = left;
    measuredRightSpeed = right;
}

void WheelController::setSmoothingFactor(float alpha)
{
    smoothingFactor = constrain(alpha, 0.01f, 1.0f);
}

float WheelController::applyDeadzoneAndExpo(float value, float expo)
{
    // Pokud je vstup pod prahem mrtvé zóny, považujeme jej za 0.
    if (fabs(value) < deadzone)
    {
        return 0;
    }
    // Aplikace expo křivky: zachování znaménka a zvýraznění nelinearity
    float sign = (value > 0) ? 1.0 : -1.0;
    return sign * pow(fabs(value), expo);
}

void WheelController::computeMotorOutputs()
{
    // Zpracování joystick vstupů s mrtvou zónou a expo křivkou
    float Y = applyDeadzoneAndExpo(joystickY, expoFwd);
    float X = applyDeadzoneAndExpo(joystickX, expoTurn);

    // Vypočtená (normalizovaná) rychlost vozíku – průměr absolutních rychlostí kol
    float absSpeed = 0.5f * (fabs(measuredLeftSpeed) + fabs(measuredRightSpeed));
    absSpeed = constrain(absSpeed, 0.0f, 1.0f);

    // Redukce účinku zatáčení dle reálné rychlosti (čím rychleji jedu, tím méně ostře zatáčím)
    float turnFactor = 1.0f - turnScaleCoefficient * absSpeed;
    turnFactor = constrain(turnFactor, 0.0f, 1.0f);
    float X_eff = X * turnFactor;

    // Diferenciální mixování
    float L_raw = Y + X_eff;
    float R_raw = Y - X_eff;

    // Normalizace rozsahu -1…+1
    float maxVal = max(fabs(L_raw), fabs(R_raw));
    if (maxVal > 1.0f)
    {
        L_raw /= maxVal;
        R_raw /= maxVal;
    }

    // Exponenciální vyhlazení (soft‑rampa)
    leftCmd = lastLeftCmd + smoothingFactor * (L_raw - lastLeftCmd);
    rightCmd = lastRightCmd + smoothingFactor * (R_raw - lastRightCmd);

    lastLeftCmd = leftCmd;
    lastRightCmd = rightCmd;
}

uint16_t WheelController::convertToDACValue(float voltage)
{
    // Conversion from voltage (MIN_VOLTAGE to MAX_VOLTAGE) to DAC value (0 to MAX_DAC_VALUE)
    return static_cast<uint16_t>((voltage / 5.0f) * MAX_DAC_VALUE);
}

void WheelController::sendToDAC(uint8_t channel, uint16_t value)
{
    // Tato funkce by měla odeslat hodnotu do DAC převodníku pomocí I2C.
    // Implementujte podle dokumentace DFR1073. Zde jen vypíšeme hodnotu do sériového monitoru.
    ESP_LOGI(TAG, "DAC channel %d: %d", channel, value);

    if (DAInitialized)
    {
        GP8413.setDACOutVoltage(value, channel);
    }
    else
    {
        ESP_LOGE(TAG, "DAC not initialized");
    }
}

void WheelController::update(float leftSpeedNorm, float rightSpeedNorm)
{
    setMeasuredSpeed(leftSpeedNorm, rightSpeedNorm);

    // Vypočítat aktuální výstupní příkazy pro motory
    computeMotorOutputs();

    // Rozdělení příkazů pro levý motor:
    // Pokud je hodnota kladná, motor jede vpřed; pokud záporná, couvá.
    float leftSpeed = fabs(leftCmd);
    float leftVoltage = leftSpeed * (MAX_VOLTAGE - MIN_VOLTAGE) + MIN_VOLTAGE; // škálování na MIN_VOLTAGE–MAX_VOLTAGE
    if (leftVoltage > MAX_VOLTAGE)
    {
        leftVoltage = MAX_VOLTAGE;
    }

    bool leftDir = (leftCmd >= 0) ? MOTOR_LEFT_INIT_DIR : !MOTOR_LEFT_INIT_DIR;

    float rightSpeed = fabs(rightCmd);
    float rightVoltage = rightSpeed * (MAX_VOLTAGE - MIN_VOLTAGE) + MIN_VOLTAGE;
    if (rightVoltage > MAX_VOLTAGE)
    {
        rightVoltage = MAX_VOLTAGE;
    }
    bool rightDir = (rightCmd >= 0) ? MOTOR_RIGHT_INIT_DIR : !MOTOR_RIGHT_INIT_DIR;

    // Převod napětí na DAC hodnoty a jejich odeslání

    uint16_t leftSpeedDAC = convertToDACValue(leftVoltage);
    uint16_t rightSpeedDAC = convertToDACValue(rightVoltage);

    // ESP_LOGI(TAG, "L: %d/%d, R: %d/%d", leftSpeedDAC, leftDir, rightSpeedDAC, rightDir);
    sendToDAC(leftSpeedDACChannel, leftSpeedDAC);
    setDirection(leftSpeedDACChannel, leftDir);
    sendToDAC(rightSpeedDACChannel, rightSpeedDAC);
    setDirection(rightSpeedDACChannel, rightDir);
}

void WheelController::setDirection(uint8_t channel, bool reverse)
{
    // Nastavení směru motoru (reverz nebo vpřed) na základě kanálu
    if (channel == leftSpeedDACChannel)
    {
        digitalWrite(MOTOR_LEFT_REVERSE_PIN, reverse ? HIGH : LOW);
    }
    else if (channel == rightSpeedDACChannel)
    {
        digitalWrite(MOTOR_RIGHT_REVERSE_PIN, reverse ? HIGH : LOW);
    }
}