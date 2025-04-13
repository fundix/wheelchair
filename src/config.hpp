#ifndef WHEELCHAIR_CONFIG_HPP
#define WHEELCHAIR_CONFIG_HPP

#define i2c_Address 0x3c
// #define SCL_PIN GPIO_NUM_9
// #define SDA_PIN GPIO_NUM_8

// 1-dopredu
// 2-do strany
// 3-OK
// 4-nahoru
// 5-dolu

#define SDA_PIN GPIO_NUM_8
#define SCL_PIN GPIO_NUM_10

// #define tx_io_num GPIO_NUM_4
// #define rx_io_num GPIO_NUM_5

#define JOYSTICK_X ADC1_CHANNEL_2
#define JOYSTICK_Y ADC1_CHANNEL_1

#define BUTTON_PIN_OK GPIO_NUM_3
#define BUTTON_PIN_UP GPIO_NUM_5
#define BUTTON_PIN_DOWN GPIO_NUM_6

#define MOTOR_LEFT_INIT_DIR LOW
#define MOTOR_LEFT_REVERSE_PIN GPIO_NUM_7
#define MOTOR_RIGHT_INIT_DIR HIGH
#define MOTOR_RIGHT_REVERSE_PIN GPIO_NUM_8

// const char *ssid = "SuicideWheelchair";
// const char *password = "SuicideWheelchair";

#endif // WHEELCHAIR_CONFIG_HPP