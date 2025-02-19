#ifndef NEVERA_FIRMWARE_PINS_HPP
#define NEVERA_FIRMWARE_PINS_HPP

#define I2C_SDA 16
#define I2C_SCL 17

#define BTN_PAIR 1

#define BATT_READ 2
#define PIN_VREF 21

#define MOTOR_A 43
#define MOTOR_B 44
#define SERVO_STEER 35

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
#define EXP_HEAD_L 10
#define EXP_HEAD_R 11
#define EXP_TAIL_L 0
#define EXP_TAIL_R 1

#endif //NEVERA_FIRMWARE_PINS_HPP