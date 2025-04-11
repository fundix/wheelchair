#ifndef WHEEL_CONTROLLER_HPP
#define WHEEL_CONTROLLER_HPP

#include <Wire.h>
#include <Arduino.h>
#include <math.h>
#include "esp_log.h"

#define MAX_DAC_VALUE 32767 // 15-bit DAC, tj. max hodnota

class MotorController
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
    MotorController();
    void begin();                            // inicializace I2C a DAC zařízení
    void setJoystickInput(float x, float y); // x: zatáčení (-1 až +1), y: plynový příkaz (-1 až +1)
    void setSpeedMode(SpeedMode mode);
    SpeedMode getSpeedMode() const { return currentMode; } // vrátí aktuální rychlostní mód
    void update();                                         // vypočítá nové výstupy a odešle je do DAC

    // Laditelné parametry – nastavitelné i v runtime
    void setTurnScaleCoefficient(float coeff);     // koeficient pro redukci efektu zatáčení při vyšší rychlosti
    void setExpoCurveFwd(float expo);              // exponent pro plyn (vpřed/vzad)
    void setExpoCurveTurn(float expo);             // exponent pro zatáčení (boční osa)
    void setDeadzone(float dz);                    // mrtvá zóna joysticku (např. 0.05)
    float getLeftCmd() const { return leftCmd; }   // vrátí aktuální příkaz pro levý motor
    float getRightCmd() const { return rightCmd; } // vrátí aktuální příkaz pro pravý motor

private:
    // Uchovává vstupy z joysticku
    float joystickX;
    float joystickY;
    SpeedMode currentMode;

    // Laditelné parametry
    float deadzone;             // např. 0.05 (5 %)
    float expoFwd;              // např. 1.5 (míra nelinearity pro vpřed/vzad)
    float expoTurn;             // např. 2.0 (míra nelinearity pro zatáčení)
    float turnScaleCoefficient; // např. 0.5 (při plném plynu se efekt zatáčení sníží o 50 %)

    // Definice DAC kanálů (předpokládáme 4 kanály: rychlost a směr pro každý motor)
    const uint8_t leftSpeedDACChannel = 0;
    const uint8_t leftDirDACChannel = 1;
    const uint8_t rightSpeedDACChannel = 2;
    const uint8_t rightDirDACChannel = 3;

    // Vypočítané příkazy motorů (rozsah -1 až +1)
    float leftCmd;
    float rightCmd;

    // Privátní metody
    void computeMotorOutputs();
    void sendToDAC(uint8_t channel, uint16_t value);
    uint16_t convertToDACValue(float voltage);
    float applyDeadzoneAndExpo(float value, float expo);
};

#endif // WHEEL_CONTROLLER_HPP