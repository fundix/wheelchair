#ifndef WHEELCHAIR_CONFIG_S3_HPP
#define WHEELCHAIR_CONFIG_S3_HPP

#define i2c_Address 0x3c

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
#define i2c_DAC_Address 0x59

// 1-dopredu
// 2-do strany
// 3-OK
// 6-nahoru
// 5-dolu

#define WIRE_SDA_PIN GPIO_NUM_12 //
#define WIRE_SCL_PIN GPIO_NUM_11 //

#define JOYSTICK_X ADC1_CHANNEL_9 // ok gpio 9
#define JOYSTICK_Y ADC1_CHANNEL_8 // ok gpio 10

#define BUTTON_PIN_OK GPIO_NUM_14   // ok
#define BUTTON_PIN_UP GPIO_NUM_15   // ok
#define BUTTON_PIN_DOWN GPIO_NUM_13 // ok

#define MOTOR_LEFT_INIT_DIR LOW
#define MOTOR_LEFT_REVERSE_PIN GPIO_NUM_7 // BUG wrong pin
#define MOTOR_RIGHT_INIT_DIR LOW
#define MOTOR_RIGHT_REVERSE_PIN GPIO_NUM_7 // BUG wrong pin

#define MIN_VOLTAGE 0.0
#define MAX_VOLTAGE 4.3

// const char *ssid = "SuicideWheelchair";
// const char *password = "SuicideWheelchair";

#endif // WHEELCHAIR_CONFIG_S3_HPP