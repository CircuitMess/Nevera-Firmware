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
#include <Services/ShutdownService.h>
#include "States/IntroState.h"
#include <Util/StateMachine/StateMachine.h>
#include "Services/Battery.h"

#include <iostream>
#include "src/lib/sprofiler/sprofiler.h"

#include <Services/Modules/ModuleService.h>

#define NUM_OF_SPIN_TASKS   6
#define SPIN_ITER           500000  //Actual CPU cycles used will depend on compiler optimization
#define SPIN_TASK_PRIO      2
#define STATS_TASK_PRIO     3
#define STATS_TICKS         pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

static esp_err_t print_real_time_stats(TickType_t xTicksToWait)
{
    TaskStatus_t *start_array = NULL, *end_array = NULL;
    UBaseType_t start_array_size, end_array_size;
    uint32_t start_run_time, end_run_time;
    esp_err_t ret;

    //Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array = static_cast<TaskStatus_t *>(malloc(sizeof(TaskStatus_t) * start_array_size));
    if (start_array == NULL) {
    	std::cout << "start_array is nullptr" << std::endl;
        ret = ESP_ERR_NO_MEM;
    	free(start_array);
    	free(end_array);
    	return ret;
    }
    //Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
    	std::cout << "start_array_size is 0" << std::endl;
        ret = ESP_ERR_INVALID_SIZE;
    	free(start_array);
    	free(end_array);
    	return ret;
    }

    vTaskDelay(xTicksToWait);

    //Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array = static_cast<TaskStatus_t *>(malloc(sizeof(TaskStatus_t) * end_array_size));
    if (end_array == NULL) {
    	std::cout << "end_array is nullptr" << std::endl;
        ret = ESP_ERR_NO_MEM;
    	free(start_array);
    	free(end_array);
    	return ret;
    }
    //Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
    	std::cout << "end_array_size is 0" << std::endl;
        ret = ESP_ERR_INVALID_SIZE;
    	free(start_array);
    	free(end_array);
    	return ret;
    }

    //Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
    	std::cout << "total_elapsed_time is 0" << std::endl;
        ret = ESP_ERR_INVALID_STATE;
    	free(start_array);
    	free(end_array);
    	return ret;
    }

    std::cout << "| Task | Run Time | Percentage\n" << std::endl;
    //Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                //Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle = NULL;
                break;
            }
        }
        //Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time = (task_elapsed_time * 100UL) / (total_elapsed_time * portNUM_PROCESSORS);
        	std::cout << "| " << start_array[i].pcTaskName << " | " << task_elapsed_time << " | " << percentage_time << "%" << std::endl;
        }
    }

    //Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
        	std::cout << "| " << start_array[i].pcTaskName << " | Deleted" << std::endl;
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
        	std::cout << "| " << end_array[i].pcTaskName << " | Created" << std::endl;
        }
    }
    ret = ESP_OK;

	free(start_array);
	free(end_array);
	return ret;
}

static void stats_task(void *arg)
{
	//Print real time stats periodically
	while (true) {
		if(print_real_time_stats(STATS_TICKS) == ESP_OK){
			printf("Real time stats obtained\n");
		}else{
			printf("Error getting real time stats\n");
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

class Nevera : public Application {
	GENERATED_BODY(Nevera, Application)

public:
	Nevera() noexcept: Super(1000, 4 * 1024, 8, 0){}

protected:
	virtual void begin() noexcept override {
		Super::begin();

		//sprofiler_initialize(100);
		//xTaskCreatePinnedToCore(stats_task, "stats", 4096, NULL, STATS_TASK_PRIO, NULL, tskNO_AFFINITY);

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

		ledService->on(RGB_LEDs::Left1, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right1, { 0.0f, 1.0f, 0.0f });

		ledService->on(RGB_LEDs::Left2, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right2, { 0.0f, 1.0f, 0.0f });

		ledService->on(RGB_LEDs::Left3, { 0.0f, 1.0f, 0.0f });
		ledService->on(RGB_LEDs::Right3, { 0.0f, 1.0f, 0.0f });

		auto i2s = registerPeriphery<I2S>(I2S_NUM_AUTO, config->getI2SConfig());

		auto audio = registerService<Audio>(i2s, newObject<AACSource>().get(), OutputPin{ gpioOut, SPKR_EN });
		audio->setGain(0.1f);

		auto pwm = registerDriver<OutputPWM>(config->getPwmOutputs());

		static const std::vector<MotorDef<int>> motorDefs = {
				{0, {gpioOut, MOTOR_B}, {pwm, 0}}
		};
		auto motors = registerService<Motors<int>>(motorDefs);

		auto mcpwm = registerDriver<OutputMCPWM>(config->getMcpwmPinDefs());

		std::vector<ServoDef<ServoEnum>> servoDefs = {
				{ ServoEnum::Steer, {0.30f, 0.60f}, { mcpwm, 0 }}
		};
		auto servos = registerService<Servos<ServoEnum>>(servoDefs);

		I2C* camI2C = registerPeriphery<I2C>(I2CPort::One, static_cast<gpio_num_t>(I2C_CAM_SDA), static_cast<gpio_num_t>(I2C_CAM_SCL));

		auto camera = registerDevice<Camera>(config->getCameraConfig(), camI2C, [](sensor_t* sensor){
			sensor->set_hmirror(sensor, 0);
			sensor->set_vflip(sensor, 0);
			sensor->set_gain_ctrl(sensor, 1);
		});

		camera->init();

		Battery* battery = registerService<Battery>(OutputPin{ gpioOut, PIN_VREF });
		battery->begin();

		battery->OnLevelChanged.bind(this, &Nevera::onBatteryChange);

		if(!SPIFFS::init()){
			return;
		}

		audio->play("/spiffs/Intro2.aac");

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		TCPClient* tcp = registerService<TCPClient>();
		tcp->OnDisconnect.bind(this, &Nevera::onDisconnect);

		registerService<UDPEmitter>();
		registerService<Comm>();

		StateMachine* stateMachine = registerService<StateMachine>(1000, 4 * 1024, 10, -1);

		stateMachine->setStartingStateType(IntroState::staticClass());
	}

	virtual void tick(float deltaTime) noexcept override {
		Super::tick(deltaTime);
	}

	virtual void onDestroy() noexcept override {
		Super::onDestroy();
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