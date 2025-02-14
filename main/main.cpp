#include <nvs_flash.h>
#include <Core/EntryPoint.h>
#include <Periphery/WiFi.h>
#include "src/Services/WiFiStation.h"
#include "src/Services/TCPClient.h"
#include "src/Services/UDPEmitter.h"
#include <Periphery/I2C.h>
#include <Devices/AW9523.h>
#include <Drivers/Output/OutputCurrAW.h>
#include "HardwareConfig.h"
#include "Pins.hpp"
#include "FileSystem/SPIFFS.h"
#include "Drivers/Output/OutputGPIO.h"
#include <Services/LED/LED.h>
#include <Services/Audio/Audio.h>
#include <Services/Audio/AACSource.h>
#include <Periphery/GPIOPeriph.h>

DECLARE_ENUM(LEDs, HeadlightsLeft, HeadlightsRight, TaillightsLeft, TaillightsRight);

DECLARE_ENUM(RGB_LEDs);

class TestApp : public Application {
	GENERATED_BODY(TestApp, Application)

protected:
	virtual void begin() noexcept override {
		Super::begin();

		auto ret = nvs_flash_init();
		if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);

		HardwareConfiguration* config = registerSingleton<HardwareConfiguration>();

		I2C* i2c = registerPeriphery<I2C>(I2CPort::Zero, static_cast<gpio_num_t>(I2C_SDA), static_cast<gpio_num_t>(I2C_SCL));

		AW9523* aw9523 = registerDevice<AW9523>(i2c, config->getAW9523Address());
		aw9523->setCurrentLimit(AW9523::IMAX);

		OutputCurrAW* outputCurrAW = registerService<OutputCurrAW>(config->getAW9523Outputs(), aw9523);
		for(const auto out: config->getAW9523Outputs()){
			outputCurrAW->write(out.port, false);
		}

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		registerService<TCPClient>();
		registerService<UDPEmitter>();

		static const std::vector<std::pair<LEDs, OutputPin>> ledPins = {
				{ LEDs::HeadlightsLeft,  { outputCurrAW, EXP_HEAD_L }},
				{ LEDs::HeadlightsRight, { outputCurrAW, EXP_HEAD_R }},
				{ LEDs::TaillightsLeft,  { outputCurrAW, EXP_TAIL_L }},
				{ LEDs::TaillightsRight, { outputCurrAW, EXP_TAIL_R }}
		};
		LED<LEDs, RGB_LEDs>* ledService = registerService<LED<LEDs, RGB_LEDs>>();

		ledService->reg(ledPins);

		auto gpio = registerPeriphery<GPIOPeriph>();
		auto gpioOut = registerDriver<OutputGPIO>(config->getGPIOOutputs(), gpio);
		auto i2s = registerPeriphery<I2S>(I2S_NUM_AUTO, config->getI2SConfig());

		auto audio = registerService<Audio>(i2s, newObject<AACSource>().get(), OutputPin{ gpioOut, SPKR_EN });
		audio->setGain(0.5f);


		if(!SPIFFS::init()){
			return;
		}

		audio->play("/spiffs/Intro2.aac");
	}

	virtual void tick(float deltaTime) noexcept override {
		Super::tick(deltaTime);
	}

	virtual void onDestroy() noexcept override {
		Super::onDestroy();
	}
};

CMF_MAIN(TestApp)