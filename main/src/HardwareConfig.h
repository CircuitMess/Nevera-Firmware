#ifndef NEVERA_FIRMWARE_HARDWARECONFIG_H
#define NEVERA_FIRMWARE_HARDWARECONFIG_H

#include <Misc/Singleton.h>
#include <driver/i2s_std.h>
#include "Drivers/Interface/OutputDriver.h"
#include "Pins.hpp"
#include <esp_camera.h>

#include <Drivers/Input/InputTouchGPIO.h>

class HardwareConfiguration : public Singleton {
	GENERATED_BODY(HardwareConfiguration, Singleton)

public:
	uint8_t getAW9523Address() const noexcept{ return AW9523Address; }

	const std::vector<OutputPinDef>& getAW9523Outputs() const noexcept{ return AW9523Outputs; }

	static const i2s_std_config_t& getI2SConfig(){ return I2S_config; }

	const std::vector<OutputPinDef>& getGPIOOutputs() const{ return GPIOOutputs; }

	const camera_config_t& getCameraConfig() const{ return cameraConfig; }

	const std::vector<TouchPinDef>& getTouchInputs() const{ return TouchInputs; }

private:
	const uint8_t AW9523Address = 0x5b;

	//ports are not inverted since AW9523 led driver is a current source
	const std::vector<OutputPinDef> AW9523Outputs = {
			{ EXP_HEAD_L, false },
			{ EXP_HEAD_R, false },
			{ EXP_TAIL_L, false },
			{ EXP_TAIL_R, false }
	};

	static constexpr i2s_std_config_t I2S_config = {
			.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(24000),
			.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
			.gpio_cfg = {
					.mclk = I2S_GPIO_UNUSED,
					.bclk = (gpio_num_t) I2S_BCK,
					.ws = (gpio_num_t) I2S_WS,
					.dout = (gpio_num_t) I2S_DATA,
					.din = I2S_GPIO_UNUSED,
					.invert_flags = {
							.mclk_inv = false,
							.bclk_inv = false,
							.ws_inv = false,
					},
			},
	};

	const std::vector<OutputPinDef> GPIOOutputs = {
			{ SPKR_EN, false }
	};

	camera_config_t cameraConfig = {
			.pin_pwdn = CAM_PIN_PWDN,                   /*!< GPIO pin for camera power down line */
			.pin_reset = CAM_PIN_RESET,                  /*!< GPIO pin for camera reset line */
			.pin_xclk = CAM_PIN_XCLK,                   /*!< GPIO pin for camera XCLK line */
			.pin_sccb_sda = -1,
			.pin_sccb_scl = -1,
			.pin_d7 = CAM_PIN_D7,                     /*!< GPIO pin for camera D7 line */
			.pin_d6 = CAM_PIN_D6,                     /*!< GPIO pin for camera D6 line */
			.pin_d5 = CAM_PIN_D5,                     /*!< GPIO pin for camera D5 line */
			.pin_d4 = CAM_PIN_D4,                     /*!< GPIO pin for camera D4 line */
			.pin_d3 = CAM_PIN_D3,                     /*!< GPIO pin for camera D3 line */
			.pin_d2 = CAM_PIN_D2,                     /*!< GPIO pin for camera D2 line */
			.pin_d1 = CAM_PIN_D1,                     /*!< GPIO pin for camera D1 line */
			.pin_d0 = CAM_PIN_D0,                     /*!< GPIO pin for camera D0 line */
			.pin_vsync = CAM_PIN_VSYNC,                  /*!< GPIO pin for camera VSYNC line */
			.pin_href = CAM_PIN_HREF,                   /*!< GPIO pin for camera HREF line */
			.pin_pclk = CAM_PIN_PCLK,                   /*!< GPIO pin for camera PCLK line */

			.xclk_freq_hz = 14000000,               /*!< Frequency of XCLK signal, in Hz. EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode */

			.ledc_timer = LEDC_TIMER_0,        /*!< LEDC timer to be used for generating XCLK  */
			.ledc_channel = LEDC_CHANNEL_0,    /*!< LEDC channel to be used for generating XCLK  */

			.pixel_format = PIXFORMAT_JPEG,       /*!< Format of the pixel data: PIXFORMAT_ + YUV422|GRAYSCALE|RGB565|JPEG  */
			.frame_size = FRAMESIZE_QQVGA,         /*!< Size of the output image: FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA  */

			.jpeg_quality = 12,               /*!< Quality of JPEG output. 0-63 lower means higher quality  */
			.fb_count = 2,                /*!< Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)  */
			.fb_location = CAMERA_FB_IN_PSRAM, /*!< The location where the frame buffer will be allocated */
			.grab_mode = CAMERA_GRAB_LATEST,   /*!< When buffers should be filled */

			.sccb_i2c_port = 0,              /*!< If pin_sccb_sda is -1, use the already configured I2C bus by number */
	};

	inline static const std::vector<TouchPinDef> TouchInputs = {
		{{BTN_PAIR, false}, 1000}
	};
};

#endif //NEVERA_FIRMWARE_HARDWARECONFIG_H
