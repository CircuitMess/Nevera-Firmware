#include "JigHWTest.h"
#include "SPIFFSChecksum.hpp"
#include <Pins.hpp>
#include <soc/efuse_reg.h>
#include <esp_efuse.h>
#include <ctime>
#include <iostream>
#include <esp_mac.h>
#include <Devices/AW9523.h>
#include <Devices/Camera.h>
#include <driver/gptimer.h>
#include <driver/ledc.h>
#include <Drivers/Output/OutputGPIO.h>
#include <Services/ADCReader.h>
#include "Util/EfuseMeta.h"
#include <Memory/ObjectMemory.h>
#include <Periphery/ADCUnit.h>
#include <Periphery/I2S.h>
#include <Services/ADCReader.h>
#include <Services/Audio/Audio.h>
#include <Services/Audio/FileAudioSource.h>

JigHWTest::JigHWTest(){
	Application* app = getApp();
	if(app == nullptr){
		printf("TEST:fail:App is null\n");
		abort();
	}

	config = app->getSingleton<HardwareConfiguration>();
	if(config == nullptr){
		printf("TEST:fail:HW config is null\n");
		abort();
	}

	i2cMaster = newObject<I2CMaster>(this, I2CPort::Zero, static_cast<gpio_num_t>(I2C_SDA), static_cast<gpio_num_t>(I2C_SCL));
	camI2cMaster = newObject<I2CMaster>(this, I2CPort::One, static_cast<gpio_num_t>(I2C_CAM_SDA), static_cast<gpio_num_t>(I2C_CAM_SCL));

	test = this;

	tests.push_back({ JigHWTest::SPIFFSTest, "SPIFFS", [](){}});
	tests.push_back({ JigHWTest::AW9523Check, "AW9523", [](){}});
	tests.push_back({ JigHWTest::CameraCheck, "Camera check", [](){}});
	tests.push_back({ JigHWTest::BatteryCheck, "Battery check", [](){}});
	tests.push_back({ JigHWTest::VoltReferenceCheck, "Voltage ref", [](){ gpio_set_level(static_cast<gpio_num_t>(PIN_VREF), 0); }});
	tests.push_back({ JigHWTest::HWVersion, "HW rev", [](){}});
}

bool JigHWTest::checkJig(){
	char buf[7];
	int wp = 0;

	const uint32_t start = millis();
	int c;
	while(millis() - start < CheckTimeout){
		vTaskDelay(1);
		c = getchar();
		if(c == EOF) continue;
		buf[wp] = static_cast<char>(c);
		wp = (wp + 1) % 7;

		for(int i = 0; i < 7; i++){
			int match = 0;
			static const char* target = "JIGTEST";

			for(int j = 0; j < 7; j++){
				match += buf[(i + j) % 7] == target[j];
			}

			if(match == 7){
				// This is important, desktop app freezes otherwise if the UART/JTAG buffer isn't emptied when it tries to write something again
				while(int c2 = getchar() != EOF) {}

				return true;
			}
		}
	}

	return false;
}


void JigHWTest::start(){
	uint64_t _chipmacid = 0LL;
	esp_efuse_mac_get_default((uint8_t*) (&_chipmacid));
	printf("\nTEST:begin:%llx\n", _chipmacid);

	bool pass = true;
	for(const Test& test : tests){
		currentTest = test.name;

		printf("TEST:startTest:%s\n", currentTest);

		const bool result = test.test();

		printf("TEST:endTest:%s\n", result ? "pass" : "fail");

		if(!(pass &= result)){
			if(test.onFail){
				test.onFail();
			}

			break;
		}
	}

	if(pass){
		printf("TEST:passall\n");
	}else{
		printf("TEST:fail:%s\n", currentTest);
	}


	//------------------------------------------------------
	bool painted = false;
	auto flashTime = 0;

	static const int LEDs[] = { EXP_HEADLIGHT_L, EXP_HEADLIGHT_R, EXP_TAILLIGHT_L, EXP_TAILLIGHT_R, EXP_LEFT1_R, EXP_LEFT1_G, EXP_LEFT2_R, EXP_LEFT2_G, EXP_LEFT3_R, EXP_LEFT3_G, EXP_RIGHT1_R, EXP_RIGHT1_G, EXP_RIGHT2_R, EXP_RIGHT2_G, EXP_RIGHT3_R, EXP_RIGHT3_G };

	StrongObjectPtr<AW9523> aw9523 = newObject<AW9523>(nullptr, *i2cMaster, config->getAW9523Address());

	for(const int LED : LEDs){
		aw9523->pinMode(LED, AW9523::LED);
	}

	StrongObjectPtr<GPIOPeriph> gpio = ApplicationStatics::getApplication()->getPeriphery<GPIOPeriph>();
	StrongObjectPtr<OutputGPIO> gpioOut = newObject<OutputGPIO>(nullptr, config->getGPIOOutputs(), gpio);
	StrongObjectPtr<I2S> i2s = newObject<I2S>(nullptr, I2S_NUM_AUTO, config->getI2SConfig());
	StrongObjectPtr<Audio> audio = newObject<Audio>(nullptr, i2s, OutputPin{ *gpioOut, SPKR_EN });
	audio->setGain(0.1f);

	for(;;){
		if(millis() - flashTime >= 500){
			if(!painted){
				for(const int LED : LEDs){
					aw9523->dim(LED, 100);
				}

				audio->play(config->getAACAudioGenerator(), std::make_unique<FileAudioSource>("/spiffs/PairSuccess.aac"));
			}else{
				for(const int LED : LEDs){
					aw9523->dim(LED, 0);
				}
			}

			painted = !painted;
			flashTime = millis();
		}
		delayMillis(10);
	}
}

void JigHWTest::log(const char* property, const char* value) const {
	printf("%s:%s:%s\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, float value) const {
	printf("%s:%s:%f\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, double value) const {
	printf("%s:%s:%lf\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, bool value) const {
	printf("%s:%s:%s\n", currentTest, property, value ? "TRUE" : "FALSE");
}

void JigHWTest::log(const char* property, uint32_t value) const {
	printf("%s:%s:%lu\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, int32_t value) const {
	printf("%s:%s:%ld\n", currentTest, property, value);
}

void JigHWTest::log(const char* property, const std::string& value) const {
	printf("%s:%s:%s\n", currentTest, property, value.c_str());
}

bool JigHWTest::AW9523Check(){
	if(i2cMaster->probe(config->getAW9523Address(), 200) != ESP_OK){
		return false;
	}

	return true;
}

bool JigHWTest::CameraCheck(){
	StrongObjectPtr<Camera> camera = newObject<Camera>(nullptr, config->getCameraConfig(), *camI2cMaster, [](sensor_t* sensor){
			sensor->set_hmirror(sensor, 0);
			sensor->set_vflip(sensor, 0);
			sensor->set_gain_ctrl(sensor, 1);
		});

	if(camera->init() != ESP_OK){
		return false;
	}

	return true;
}

bool JigHWTest::BatteryCheck(){
	//Just in case BattVref was active
	constexpr gpio_num_t RefSwitch = (gpio_num_t) PIN_VREF;
	gpio_set_direction(RefSwitch, GPIO_MODE_OUTPUT);
	gpio_set_level(RefSwitch, 0);
	delayMillis(100);

	constexpr adc_oneshot_chan_cfg_t cfg = {
		.atten = ADC_ATTEN_DB_2_5,
		.bitwidth = ADC_BITWIDTH_12
	};

	static constexpr float Factor = 4.0f;
	static constexpr float Offset = 0;

	const StrongObjectPtr<ADCReader> reader = newObject<ADCReader>(nullptr, (gpio_num_t) PIN_BATT, cfg, true, newObject<FactorOffset_ADCFilter>(nullptr, Factor, Offset).get());

	static constexpr uint16_t numReadings = 50;
	static constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		reading += reader->sample();
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	test->log("reading", reading);

	if(reading < BatVoltageMinimum){
		return false;
	}

	return true;
}

bool JigHWTest::VoltReferenceCheck(){
	constexpr gpio_num_t RefSwitch = static_cast<gpio_num_t>(PIN_VREF);
	gpio_set_direction(RefSwitch, GPIO_MODE_OUTPUT);
	gpio_set_level(RefSwitch, 1);
	delayMillis(100);

	constexpr adc_oneshot_chan_cfg_t cfg = {
		.atten = ADC_ATTEN_DB_2_5,
		.bitwidth = ADC_BITWIDTH_12
	};

	static constexpr float Factor = 4.0f;
	static constexpr float Offset = 0;

	const StrongObjectPtr<ADCReader> reader = newObject<ADCReader>(nullptr, (gpio_num_t) PIN_BATT, cfg, true, newObject<FactorOffset_ADCFilter>(nullptr, Factor, Offset).get());

	constexpr uint16_t numReadings = 50;
	constexpr uint16_t readDelay = 10;
	uint32_t reading = 0;

	for(int i = 0; i < numReadings; i++){
		reading += reader->sample();
		vTaskDelay(readDelay / portTICK_PERIOD_MS);
	}
	reading /= numReadings;

	test->log("reading", reading);

	if(reading < VoltReference - VoltReferenceTolerance || reading > VoltReference + VoltReferenceTolerance){
		return false;
	}

	gpio_set_level(RefSwitch, 0);

	return true;
}

bool JigHWTest::SPIFFSTest(){
	if(esp_vfs_spiffs_register(&spiffsConfig) != ESP_OK){
		test->log("spiffs", false);
		return false;
	}

	for(const auto& f : SPIFFSChecksums){
		const auto file = fopen(f.name, "rb");
		if(file == nullptr){
			test->log("missing", f.name);
			return false;
		}

		const uint32_t sum = calcChecksum(file);
		fclose(file);

		if(sum != f.sum){
			test->log("file", f.name);
			test->log("expected", (uint32_t) f.sum);
			test->log("got", (uint32_t) sum);

			return false;
		}
	}

	return true;
}

uint32_t JigHWTest::calcChecksum(FILE* file){
	if(file == nullptr) return 0;

#define READ_SIZE 512

	uint32_t sum = 0;
	uint8_t b[READ_SIZE];
	size_t read = 0;
	while((read = fread(b, 1, READ_SIZE, file))){
		for(int i = 0; i < read; i++){
			sum += b[i];
		}
	}

	return sum;
}

bool JigHWTest::HWVersion(){
	uint16_t version = 0;
	bool result = EfuseMeta::readPID(version);

	if(!result){
		test->log("HW version", "couldn't PID read from efuse");
		return false;
	}

	if(version != 0){
		test->log("Existing HW version", static_cast<uint32_t>(version));

		if(version == EfuseMeta::getHardcodedPID()){
			test->log("Already fused.", static_cast<uint32_t>(version));
			return true;
		}else{
			test->log("Wrong binary already fused!", static_cast<uint32_t>(version));
			return false;
		}
	}

	return EfuseMeta::write();
}
