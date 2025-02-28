#ifndef NEVERA_FIRMWARE_PINS_HPP
#define NEVERA_FIRMWARE_PINS_HPP

#define I2C_SDA 35
#define I2C_SCL 36

#define I2C_CAM_SDA 17
#define I2C_CAM_SCL 16

#define BTN_PAIR 1

#define PIN_BATT 2
#define PIN_VREF 21

#define MOTOR_A 37
#define MOTOR_B 38

#define SERVO_STEER 39

#define SPKR_EN 47
#define I2S_DATA 33
#define I2S_BCK 34
#define I2S_WS 48

#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_PWDN    14
#define CAM_PIN_XCLK    11
#define CAM_PIN_D7		12
#define CAM_PIN_D6		10
#define CAM_PIN_D5		9
#define CAM_PIN_D4		7
#define CAM_PIN_D3		5
#define CAM_PIN_D2		3
#define CAM_PIN_D1		4
#define CAM_PIN_D0		6
#define CAM_PIN_VSYNC	15
#define CAM_PIN_HREF	13
#define CAM_PIN_PCLK	8

// AW9523 pins:
#define EXP_HEADLIGHT_L 9
#define EXP_HEADLIGHT_R 8
#define EXP_TAILLIGHT_L 4
#define EXP_TAILLIGHT_R 5
#define EXP_LEFT1_R 11
#define EXP_LEFT1_G 10
#define EXP_LEFT2_R 1
#define EXP_LEFT2_G 0
#define EXP_LEFT3_R 3
#define EXP_LEFT3_G 2
#define EXP_RIGHT1_R 15
#define EXP_RIGHT1_G 14
#define EXP_RIGHT2_R 13
#define EXP_RIGHT2_G 12
#define EXP_RIGHT3_R 7
#define EXP_RIGHT3_G 6

#endif //NEVERA_FIRMWARE_PINS_HPP