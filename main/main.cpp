#include <nvs_flash.h>
#include <Core/EntryPoint.h>
#include <Periphery/WiFi.h>
#include "src/Services/WiFiStation.h"
#include "src/Services/TCPClient.h"
#include "src/Services/UDPEmitter.h"
#include <Periphery/I2CMaster.h>
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
#include <Periphery/GPIOPeriph.h>
#include <Devices/Camera.h>
#include <Services/Motors/Motors.h>
#include <Drivers/Input/InputTouchGPIO.h>
#include <Services/ButtonInput.h>
#include "Enums.h"
#include <Services/Feed.h>
#include <Services/ShutdownService.h>
#include <Services/Audio/FileAudioSource.h>
#include <Util/stdafx.h>
#include "States/IntroState.h"
#include <Util/StateMachine/StateMachine.h>
#include "Services/Battery.h"
#include "Util/EfuseMeta.h"
#include "JigHWTest/JigHWTest.h"

class Nevera : public Application {
	GENERATED_BODY(Nevera, Application, void)

public:
	Nevera() noexcept: Super(1000, 4 * 1024, 8, 0){}

protected:
	virtual void begin() noexcept override {
		auto ret = nvs_flash_init();
		if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);

		HardwareConfiguration* config = registerSingleton<HardwareConfiguration>();
		GPIOPeriph* gpio = registerPeriphery<GPIOPeriph>();

		if(JigHWTest::checkJig()){
			printf("Jig\n");
			StrongObjectPtr<JigHWTest> test = newObject<JigHWTest>();
			test->start();
			vTaskDelete(nullptr);
		}else{
			printf("Hello\n");
		}

		if(!EfuseMeta::check()){
			while(true){
				vTaskDelay(1000);
				EfuseMeta::log();
			}
		}

		InputTouchGPIO* touch = registerDriver<InputTouchGPIO>(config->getTouchInputs());

		static const std::vector<std::pair<Enum<int>, InputPin>> ButtonInputs = {
			{ Button::Pair, { touch, BTN_PAIR }}
		};

		ButtonInput* buttonInput = registerService<ButtonInput>();
		buttonInput->reg(ButtonInputs);

		auto gpioOut = registerDriver<OutputGPIO>(config->getGPIOOutputs(), gpio);

		I2CMaster* i2cMaster = registerPeriphery<I2CMaster>(I2CPort::Zero, static_cast<gpio_num_t>(I2C_SDA), static_cast<gpio_num_t>(I2C_SCL));

		AW9523* aw9523 = registerDevice<AW9523>(i2cMaster, config->getAW9523Address());
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

		ledService->on(RGB_LEDs::Left1, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right1, { 0.0f, 1.0f, 0.0f });

		ledService->on(RGB_LEDs::Left2, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right2, { 0.0f, 1.0f, 0.0f });

		ledService->on(RGB_LEDs::Left3, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right3, { 0.0f, 1.0f, 0.0f });

		auto i2s = registerPeriphery<I2S>(I2S_NUM_AUTO, config->getI2SConfig());

		auto audio = registerService<Audio>(i2s, OutputPin{ gpioOut, SPKR_EN });
		audio->setGain(0.1f);

		auto pwm = registerDriver<OutputPWM>(config->getPwmOutputs());

		static const std::vector<MotorDef<int>> motorDefs = {
				{0, {gpioOut, MOTOR_B}, {pwm, 0}}
		};
		auto motors = registerService<Motors<int>>(motorDefs);

		auto mcpwm = registerDriver<OutputMCPWM>(config->getMcpwmPinDefs());

		std::vector<ServoDef<ServoEnum>> servoDefs = {
				{ ServoEnum::Steer, {0.30f, 0.55f}, { mcpwm, 0 }}
		};
		auto servos = registerService<Servos<ServoEnum>>(servoDefs);

		I2CMaster* camI2CMaster = registerPeriphery<I2CMaster>(I2CPort::One, static_cast<gpio_num_t>(I2C_CAM_SDA), static_cast<gpio_num_t>(I2C_CAM_SCL));

		auto camera = registerDevice<Camera>(config->getCameraConfig(), camI2CMaster, [](sensor_t* sensor){
			sensor->set_hmirror(sensor, 0);
			sensor->set_vflip(sensor, 0);
			sensor->set_gain_ctrl(sensor, 1);
		});

		camera->init();

		registerService<Feed>();

		Battery* battery = registerService<Battery>(OutputPin{ gpioOut, PIN_VREF });
		battery->OnLevelChanged.bind(this, &Nevera::onBatteryChange);
		battery->begin();
		onBatteryChange(battery->getLevel());

		if(!SPIFFS::init()){
			return;
		}

		audio->play(config->getAACAudioGenerator(), std::make_unique<FileAudioSource>("/spiffs/Intro2.aac"));

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		TCPClient* tcp = registerService<TCPClient>();
		tcp->OnDisconnect.bind(this, &Nevera::onDisconnect);

		registerService<UDPEmitter>();
		registerService<Comm>();

		StateMachine* stateMachine = registerService<StateMachine>(100, 4 * 1024, 8, 1);

		stateMachine->setStartingStateType(IntroState::staticClass());
	}


private:
	void onBatteryChange(Battery::Level level) const noexcept{
		if(level != Battery::Level::Critical) {
			return;
		}

		ShutdownService::shutdown(ShutdownReason::Battery);
		vTaskDelay(portMAX_DELAY);
	}

	void onDisconnect() const noexcept {
		if(Servos<ServoEnum>* servos = getService<Servos<ServoEnum>>()) {
			servos->set(ServoEnum::Steer, 0.5f);
		}

		if(Motors<int>* motors = getService<Motors<int>>()) {
			motors->set(0, 0.0f);
		}
	}
};

CMF_MAIN(Nevera)