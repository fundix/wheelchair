#ifndef WHEEL_CONTROLLER_HPP
#define WHEEL_CONTROLLER_HPP

#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include "esp_log.h"
#include <DFRobot_GP8XXX.h>
#include "config.hpp"

#define MAX_DAC_VALUE 32767 // 15-bit DAC, tj. max hodnota

class WheelController
{
public:
    // Rychlostní módy pro omezení maximálního výkonu
    enum SpeedMode
    {
        SPEED_LOW,
        SPEED_MEDIUM,
        SPEED_HIGH
    };

    // Konstruktor a inicializace
    WheelController();
    void begin();                            // inicializace I2C a DAC zařízení
    void setJoystickInput(float x, float y); // x: zatáčení (-1 až +1), y: plynový příkaz (-1 až +1)
    void setSpeedMode(SpeedMode mode);
    SpeedMode getSpeedMode() const { return currentMode; }                // vrátí aktuální rychlostní mód
    void update(float leftSpeedNorm = 0.0f, float rightSpeedNorm = 0.0f); // přijme aktuální rychlosti kol

    // Laditelné parametry – nastavitelné i v runtime
    void setTurnScaleCoefficient(float coeff);      // koeficient pro redukci efektu zatáčení při vyšší rychlosti
    void setExpoCurveFwd(float expo);               // exponent pro plyn (vpřed/vzad)
    void setExpoCurveTurn(float expo);              // exponent pro zatáčení (boční osa)
    void setDeadzone(float dz);                     // mrtvá zóna joysticku (např. 0.05)
    void setSmoothingFactor(float alpha);           // 0 < alpha ≤ 1
    void setMeasuredSpeed(float left, float right); // normalised -1…+1
    float getLeftCmd() const { return leftCmd; }    // vrátí aktuální příkaz pro levý motor
    float getRightCmd() const { return rightCmd; }  // vrátí aktuální příkaz pro pravý motor

private:
    // Uchovává vstupy z joysticku
    bool DAInitialized = false;
    DFRobot_GP8413 GP8413;
    float joystickX;
    float joystickY;
    SpeedMode currentMode;

    // Laditelné parametry
    float deadzone;             // např. 0.05 (5 %)
    float expoFwd;              // např. 1.5 (míra nelinearity pro vpřed/vzad)
    float expoTurn;             // např. 2.0 (míra nelinearity pro zatáčení)
    float turnScaleCoefficient; // např. 0.5 (při plném plynu se efekt zatáčení sníží o 50 %)

    // Feedback & filtr
    float measuredLeftSpeed; // normalised -1…+1
    float measuredRightSpeed;
    float smoothingFactor; // expo‑smoothing pro povely (0…1)
    float lastLeftCmd;
    float lastRightCmd;

    // Definice DAC kanálů (předpokládáme 4 kanály: rychlost a směr pro každý motor)
    const uint8_t leftSpeedDACChannel = 0;
    // const uint8_t leftDirDACChannel = 1;
    const uint8_t rightSpeedDACChannel = 1;
    // const uint8_t rightDirDACChannel = 3;

    // Vypočítané příkazy motorů (rozsah -1 až +1)
    float leftCmd;
    float rightCmd;

    // Privátní metody
    void computeMotorOutputs();
    void sendToDAC(uint8_t channel, uint16_t value);
    void setDirection(uint8_t channel, bool reverse);
    uint16_t convertToDACValue(float voltage);
    float applyDeadzoneAndExpo(float value, float expo);
};

#endif // WHEEL_CONTROLLER_HPP