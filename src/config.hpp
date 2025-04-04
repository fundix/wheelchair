#ifndef WHEELCHAIR_CONFIG_HPP
#define WHEELCHAIR_CONFIG_HPP

#define i2c_Address 0x3c
// #define SCL_PIN GPIO_NUM_9
// #define SDA_PIN GPIO_NUM_8

#define SDA_PIN GPIO_NUM_8
#define SCL_PIN GPIO_NUM_10

#define tx_io_num GPIO_NUM_4
#define rx_io_num GPIO_NUM_5

#define JOYSTICK_X ADC1_CHANNEL_1
#define JOYSTICK_Y ADC1_CHANNEL_2

#define BUTTON_PIN_1 GPIO_NUM_6
#define BUTTON_PIN_2 GPIO_NUM_7

#endif // WHEELCHAIR_CONFIG_HPP