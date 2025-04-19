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
#define i2c_DAC_Address 0x58

// 1-dopredu
// 2-do strany
// 3-OK
// 6-nahoru
// 5-dolu

#define SDA_PIN GPIO_NUM_8  // BUG wrong pin
#define SCL_PIN GPIO_NUM_10 // BUG wrong pin

#define JOYSTICK_X ADC1_CHANNEL_2 // BUG wrong pin
#define JOYSTICK_Y ADC1_CHANNEL_1 // BUG wrong pin

#define BUTTON_PIN_OK GPIO_NUM_3   // BUG wrong pin
#define BUTTON_PIN_UP GPIO_NUM_5   // BUG wrong pin
#define BUTTON_PIN_DOWN GPIO_NUM_6 // BUG wrong pin

#define MOTOR_LEFT_INIT_DIR LOW            // BUG wrong pin
#define MOTOR_LEFT_REVERSE_PIN GPIO_NUM_7  // BUG wrong pin
#define MOTOR_RIGHT_INIT_DIR HIGH          // BUG wrong pin
#define MOTOR_RIGHT_REVERSE_PIN GPIO_NUM_7 // BUG wrong pin
#define MIN_VOLTAGE 0.5
#define MAX_VOLTAGE 4.5

// const char *ssid = "SuicideWheelchair";
// const char *password = "SuicideWheelchair";

#endif // WHEELCHAIR_CONFIG_S3_HPP