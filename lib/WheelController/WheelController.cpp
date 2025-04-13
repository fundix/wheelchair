#include "WheelController.hpp"

static const char *TAG = "WheelController";

/**************************
----------------------------
| A2 |  A1 | A0 | i2c_addr |
----------------------------
| 0  |  0  | 0  |   0x58   |
----------------------------
| 0  |  0  | 1  |   0x59   |
----------------------------
| 0  |  1  | 0  |   0x5A   |
----------------------------
| 0  |  1  | 1  |   0x5B   |
----------------------------
| 1  |  0  | 0  |   0x5C   |
----------------------------
| 1  |  0  | 1  |   0x5D   |
----------------------------
| 1  |  1  | 0  |   0x5E   |
----------------------------
| 1  |  1  | 1  |   0x5F   |
----------------------------
***************************/

MotorController::MotorController()
    : joystickX(0), joystickY(0), currentMode(SPEED_HIGH),
      deadzone(0.05), expoFwd(1.3), expoTurn(1.2), turnScaleCoefficient(0.75),
      leftCmd(0), rightCmd(0)
{
}

void MotorController::begin()
{
    // Inicializace I2C – dle použité knihovny a DAC modulu
    // Wire.begin();
    // Inicializace DAC (specificky pro DFR1073) zde
    // např. dac.begin();
    DFRobot_GP8413 GP8413(/*deviceAddr=*/0x58);
    uint8_t retries = 0;
    const uint8_t MAX_RETRIES = 5;
    bool DAInitialized = false;

    while (!DAInitialized && retries < MAX_RETRIES)
    {
        if (GP8413.begin() == 0)
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
}

void MotorController::setJoystickInput(float x, float y)
{
    joystickX = x;
    joystickY = y;
}

void MotorController::setSpeedMode(SpeedMode mode)
{
    currentMode = mode;
}

void MotorController::setTurnScaleCoefficient(float coeff)
{
    turnScaleCoefficient = coeff;
}

void MotorController::setExpoCurveFwd(float expo)
{
    expoFwd = expo;
}

void MotorController::setExpoCurveTurn(float expo)
{
    expoTurn = expo;
}

void MotorController::setDeadzone(float dz)
{
    deadzone = dz;
}

float MotorController::applyDeadzoneAndExpo(float value, float expo)
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

void MotorController::computeMotorOutputs()
{
    // Zpracování joystick vstupů s mrtvou zónou a expo křivkou
    float Y = applyDeadzoneAndExpo(joystickY, expoFwd);
    float X = applyDeadzoneAndExpo(joystickX, expoTurn);

    // Redukce efektu zatáčení při vyšší rychlosti:
    float absY = fabs(Y);
    float turnFactor = 1 - turnScaleCoefficient * absY; // čím vyšší rychlost, tím menší účinek zatáčení
    float X_eff = X * turnFactor;

    // Diferenciální mixování
    float L = Y + X_eff;
    float R = Y - X_eff;

    // Zajištění rozsahu mezi -1 a +1
    float maxVal = max(fabs(L), fabs(R));
    if (maxVal > 1.0)
    {
        L /= maxVal;
        R /= maxVal;
    }
    leftCmd = L;
    rightCmd = R;

    // ESP_LOGI(TAG, "L: %.2f, R: %.2f", leftCmd, rightCmd);
}

uint16_t MotorController::convertToDACValue(float voltage)
{
    // Převod z napětí (0 až 5V) na hodnotu DAC (0 až MAX_DAC_VALUE)
    if (voltage < 0)
        voltage = 0;
    if (voltage > 5)
        voltage = 5;
    return (uint16_t)((voltage / 5.0) * MAX_DAC_VALUE);
}

void MotorController::sendToDAC(uint8_t channel, uint16_t value)
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

void MotorController::update()
{
    // Vypočítat aktuální výstupní příkazy pro motory
    computeMotorOutputs();

    // Rozdělení příkazů pro levý motor:
    // Pokud je hodnota kladná, motor jede vpřed; pokud záporná, couvá.
    float leftSpeed = fabs(leftCmd);
    float leftVoltage = leftSpeed * 5.0; // škálování na 5V
    // Pro levý motor: true = vpřed, false = reverz
    bool leftDir = (leftCmd >= 0);

    // U pravého motoru – vzhledem k fyzické orientaci:
    float rightSpeed = fabs(rightCmd);
    float rightVoltage = rightSpeed * 5.0;
    // U pravého motoru, pokud se požaduje vpřed (kladný příkaz), je potřeba zapnout reverz
    bool rightDir = (rightCmd < 0);

    // Aplikace rychlostního módu (škálování podle Low/Med/High)
    // float modeFactor = 1.0;
    // switch (currentMode)
    // {
    // case SPEED_LOW:
    //     modeFactor = 0.5;
    //     break;
    // case SPEED_MEDIUM:
    //     modeFactor = 0.75;
    //     break;
    // case SPEED_HIGH:
    //     modeFactor = 1.0;
    //     break;
    // }
    // leftVoltage *= modeFactor;
    // rightVoltage *= modeFactor;

    // Převod napětí na DAC hodnoty a jejich odeslání

    uint16_t leftSpeedDAC = convertToDACValue(leftVoltage);
    // uint16_t leftDirDAC = convertToDACValue(leftDir);
    uint16_t rightSpeedDAC = convertToDACValue(rightVoltage);
    // uint16_t rightDirDAC = convertToDACValue(rightDir);

    // ESP_LOGI(TAG, "L: %d/%d, R: %d/%d", leftSpeedDAC, leftDir, rightSpeedDAC, rightDir);
    return;

    sendToDAC(leftSpeedDACChannel, leftSpeedDAC);
    // sendToDAC(leftDirDACChannel, leftDirDAC);
    sendToDAC(rightSpeedDACChannel, rightSpeedDAC);
    // sendToDAC(rightDirDACChannel, rightDirDAC);
}
