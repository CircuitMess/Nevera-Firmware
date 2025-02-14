#ifndef NEVERA_FIRMWARE_HARDWARECONFIG_H
#define NEVERA_FIRMWARE_HARDWARECONFIG_H

#include <Misc/Singleton.h>
#include <driver/i2s_std.h>
#include "Drivers/Interface/OutputDriver.h"
#include "Pins.hpp"

class HardwareConfiguration : public Singleton {
	GENERATED_BODY(HardwareConfiguration, Singleton)

public:
	uint8_t getAW9523Address() const noexcept{ return AW9523Address; }

	const std::vector<OutputPinDef>& getAW9523Outputs() const noexcept{ return AW9523Outputs; }

	static const i2s_std_config_t& getI2SConfig(){ return I2S_config; }

	const std::vector<OutputPinDef>& getGPIOOutputs() const{ return GPIOOutputs; }

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

};

#endif //NEVERA_FIRMWARE_HARDWARECONFIG_H
