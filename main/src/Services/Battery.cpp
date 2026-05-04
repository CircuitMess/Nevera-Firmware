#include "Battery.h"
#include "../Pins.hpp"
#include "Memory/ObjectMemory.h"
#include "TCPClient.h"
#include "Enums.h"
#include <Services/Motors/Motors.h>
#include <Services/Motors/Servos.h>
#include <Util/stdafx.h>
#include <driver/gpio.h>


DEFINE_LOG(Battery)

float NeveraOffsets_ADCFilter::apply(float sample){
	auto* wifi = getApp()->getService<WiFiStation>();
	if(wifi){
		const bool active = wifi->getState() != WiFiStation::State::Disconnected;
		if(active){
			sample += 230; // 230 mV for WiFi
		}
	}

	auto* motors = getApp()->getService<Motors<int>>();
	if(motors){
		const float val = std::abs(motors->get(0));
		if(val >= 50){
			sample += 200; // 200mV for motors when not stopped (300mV when stopped)
		}
	}

	auto* servos = getApp()->getService<Servos<ServoEnum>>();
	if(servos){
		const float val = std::abs(servos->get(0));
		if(val <= 0.2 || val >= 0.8){
			sample += 100; // 100mV for servo
		}
	}

	sample += 150; // 150mV for LEDs

	return sample;
}

Battery::Battery(OutputPin refSwitch) : refSwitch(refSwitch), hysteresis({ 0, 4, 15, 30, 50, 70, 90, 100 }, 3){
	adc_oneshot_chan_cfg_t cfg = {
			.atten = ADC_ATTEN_DB_2_5,
			.bitwidth = ADC_BITWIDTH_12
	};

	readerBattoffsetFilter = newObject<FactorOffset_ADCFilter>(this, Factor, Offset);
	readerBattEMAFilter = newObject<EMA_ADCFilter>(this, EmaA);
	readerNeveraFilter = newObject<NeveraOffsets_ADCFilter>(this);
	std::vector<StrongObjectPtr<ADCFilter>> filters = {
			StrongObjectPtr<ADCFilter>{ readerBattoffsetFilter },
			StrongObjectPtr<ADCFilter>{ readerNeveraFilter },
			StrongObjectPtr<ADCFilter>{ readerBattEMAFilter },
			StrongObjectPtr<ADCFilter>{ newObject<Remap_ADCFilter>(this, VoltEmpty, VoltFull) }
	};

	readerBatt = newObject<ADCReader>(this, (gpio_num_t) PIN_BATT, cfg, true, newObject<Composite_ADCFilter>(this, filters).get());

	readerRef = newObject<ADCReader>(this, (gpio_num_t) PIN_BATT, cfg, true, newObject<FactorOffset_ADCFilter>(this, Factor, Offset).get());

	calibrate();

	sample(true);

	batThread = newObject<Threaded>(this, [this](){ tick(); }, "Battery", MeasureIntverval, 3 * 1024, 5, 1);
}

float Battery::getPerc() const{
	return readerBatt->getValue();
}

Battery::Level Battery::getLevel() const{
	return (Level) hysteresis.get();
}

bool Battery::isShutdown() const{
	return shutdown;
}

void Battery::calibrate(){
	refSwitch.driver->write(refSwitch.port, true);

	delayMillis(100);
	for(int i = 0; i < CalReads; i++){
		readerRef->sample();
		delayMillis(10);
	}

	float total = 0;
	for(int i = 0; i < CalReads; i++){
		total += readerRef->sample();
		delayMillis(10);
	}

	const float reading = total / (float) CalReads;
	const float offset = CalExpected - reading;
	readerBattoffsetFilter->setOffset(readerBattoffsetFilter->getOffset() + offset);

	refSwitch.driver->write(refSwitch.port, false);

	CMF_LOG(Battery, LogLevel::Info, "Calibration: Read %.02f mV, expected %.02f mV. Applying %.02f mV offset.\n", reading, CalExpected, offset);
}

void Battery::sample(bool fresh){
	if(shutdown) return;

	auto oldLevel = getLevel();

	if(fresh){
		readerBattEMAFilter->reset();
		float value = readerBatt->sample();
		hysteresis.reset(value);
		//TRACE_LOG("%f", value);
	}else{
		float value = readerBatt->sample();
		hysteresis.update(value);
		//TRACE_LOG("%f", value);
	}


	if(oldLevel != getLevel() || fresh){
		OnLevelChanged.broadcast(getLevel());
	}

	if(getLevel() == Level::Critical){
		shutdown = true;
		return;
	}
}

void Battery::tick() noexcept{
	if(shutdown) return;
	sample();
}

void Battery::begin(){
	//batThread->start();
}
