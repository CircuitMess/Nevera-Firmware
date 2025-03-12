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
#include "Services/Comm.h"
#include "Services/Motors/Servos.h"
#include <Services/LED/LED.h>
#include <Services/Audio/Audio.h>
#include <Services/Audio/AACSource.h>
#include <Periphery/GPIOPeriph.h>
#include <Devices/Camera.h>
#include <Services/Motors/Motors.h>
#include <Drivers/Input/InputTouchGPIO.h>
#include <Services/ButtonInput.h>
#include "Enums.h"
#include "States/IntroState.h"
#include <Util/StateMachine/StateMachine.h>
#include "Services/Battery.h"

class Nevera : public Application {
	GENERATED_BODY(Nevera, Application)

public:
	Nevera() noexcept: Super(1000, 4 * 1024, 8, 0){}

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

		GPIOPeriph* gpio = registerPeriphery<GPIOPeriph>();
		InputTouchGPIO* touch = registerDriver<InputTouchGPIO>(config->getTouchInputs());

		static const std::vector<std::pair<Enum<int>, InputPin>> ButtonInputs = {
			{ Button::Pair, { touch, BTN_PAIR }}
		};

		ButtonInput* buttonInput = registerService<ButtonInput>();
		buttonInput->reg(ButtonInputs);

		auto gpioOut = registerDriver<OutputGPIO>(config->getGPIOOutputs(), gpio);

		I2C* i2c = registerPeriphery<I2C>(I2CPort::Zero, static_cast<gpio_num_t>(I2C_SDA), static_cast<gpio_num_t>(I2C_SCL));

		AW9523* aw9523 = registerDevice<AW9523>(i2c, config->getAW9523Address());
		aw9523->setCurrentLimit(AW9523::IMAX_1Q);

		OutputCurrAW* outputCurrAW = registerService<OutputCurrAW>(config->getAW9523Outputs(), aw9523);
		for(const auto out: config->getAW9523Outputs()){
			outputCurrAW->write(out.port, false);
		}


		static const std::vector<std::pair<LEDs, OutputPin>> ledPins = {
				{ LEDs::HeadlightsLeft,  { outputCurrAW, EXP_HEADLIGHT_L }},
				{ LEDs::HeadlightsRight, { outputCurrAW, EXP_HEADLIGHT_R }},
				{ LEDs::TaillightsLeft,  { outputCurrAW, EXP_TAILLIGHT_L }},
				{ LEDs::TaillightsRight, { outputCurrAW, EXP_TAILLIGHT_R }}
		};

		static const std::vector<std::tuple<RGB_LEDs, OutputPin, OutputPin, OutputPin>> rgbleds = {
				{ RGB_LEDs::Left1, { outputCurrAW, EXP_LEFT1_R },  { outputCurrAW, EXP_LEFT1_G },  {}},
				{ RGB_LEDs::Left2, { outputCurrAW, EXP_LEFT2_R },  { outputCurrAW, EXP_LEFT2_G },  {}},
				{ RGB_LEDs::Left3, { outputCurrAW, EXP_LEFT3_R },  { outputCurrAW, EXP_LEFT3_G },  {}},
				{ RGB_LEDs::Right1, { outputCurrAW, EXP_RIGHT1_R }, { outputCurrAW, EXP_RIGHT1_G }, {}},
				{ RGB_LEDs::Right2, { outputCurrAW, EXP_RIGHT2_R }, { outputCurrAW, EXP_RIGHT2_G }, {}},
				{ RGB_LEDs::Right3, { outputCurrAW, EXP_RIGHT3_R }, { outputCurrAW, EXP_RIGHT3_G }, {}},
		};

		LED<LEDs, RGB_LEDs>* ledService = registerService<LED<LEDs, RGB_LEDs>>();

		ledService->reg(ledPins);
		ledService->reg(rgbleds);

		auto i2s = registerPeriphery<I2S>(I2S_NUM_AUTO, config->getI2SConfig());

		auto audio = registerService<Audio>(i2s, newObject<AACSource>().get(), OutputPin{ gpioOut, SPKR_EN });
		audio->setGain(0.1f);

		auto pwm = registerDriver<OutputPWM>(config->getPwmOutputs());

		static const std::vector<MotorDef<int>> motorDefs = {
				{0, {gpioOut, MOTOR_B}, {pwm, 0}}
		};
		auto motors = registerService<Motors<int>>(motorDefs, newObject<LinearEaser>().get());

		auto mcpwm = registerDriver<OutputMCPWM>(config->getMcpwmPinDefs());

		std::vector<ServoDef<ServoEnum>> servoDefs = {
				{ ServoEnum::Steer, { mcpwm, 0 }}
		};
		auto servos = registerService<Servos<ServoEnum>>(servoDefs, newObject<LinearEaser>(nullptr, 1.0f).get());

		I2C* camI2C = registerPeriphery<I2C>(I2CPort::One, static_cast<gpio_num_t>(I2C_CAM_SDA), static_cast<gpio_num_t>(I2C_CAM_SCL));

		auto camera = registerDevice<Camera>(config->getCameraConfig(), camI2C, [](sensor_t* sensor){
			sensor->set_hmirror(sensor, 0);
			sensor->set_vflip(sensor, 0);
			sensor->set_gain_ctrl(sensor, 1);
		});

		camera->init();

		Battery* battery = registerService<Battery>(OutputPin{ gpioOut, PIN_VREF });
		battery->begin();

		if(!SPIFFS::init()){
			return;
		}

		audio->play("/spiffs/Intro2.aac");

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		registerService<TCPClient>();
		registerService<UDPEmitter>();
		registerService<Comm>();

		StateMachine* stateMachine = registerService<StateMachine>(100, 4 * 1024, 8, 0);

		stateMachine->setStartingStateType(IntroState::staticClass());
	}

	virtual void tick(float deltaTime) noexcept override {
		Super::tick(deltaTime);
		vTaskDelay(portMAX_DELAY);
	}

	virtual void onDestroy() noexcept override {
		Super::onDestroy();
	}
};

CMF_MAIN(Nevera)