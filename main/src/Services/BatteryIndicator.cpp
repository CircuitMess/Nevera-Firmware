#include "BatteryIndicator.h"
#include "Services/LED/LEDBlinkFunction.h"
#include <utility>

BatteryIndicator::BatteryIndicator(StrongObjectPtr<Battery> battery, StrongObjectPtr<LED<LEDs, RGB_LEDs>> leds) :
		AsyncEntity(UpdatePeriod, 4 * 1024, 1, 1), battery(std::move(battery)), leds(std::move(leds)){
}

void BatteryIndicator::tick(float deltaTime) noexcept{
	AsyncEntity::tick(deltaTime);

	if(battery == nullptr){
		return;
	}

	const auto perc = battery->getPerc();

	if(perc <= ShortBlinkThreshold){
		gotoState(State::ShortBlink, perc);
	}else if(perc <= LongBlinkThreshold){
		gotoState(State::Longblink, perc);
	}else{
		gotoState(State::Continuous, perc);
	}
}

void BatteryIndicator::gotoState(BatteryIndicator::State newState, uint8_t perc){
	if(this->state == newState && this->state != State::Continuous) return;

	this->state = newState;

	switch(newState){
		case State::Continuous:{
			auto levels = BatteryIndicator::getLevels(perc);

			leds->on(RGB_LEDs::Left1, { 0, levels.green * GreenFactor, 0 });
			leds->on(RGB_LEDs::Right1, { 0, levels.green * GreenFactor, 0 });

			leds->on(RGB_LEDs::Left2, { levels.orange, levels.orange * GreenFactor, 0 });
			leds->on(RGB_LEDs::Right2, { levels.orange, levels.orange * GreenFactor, 0 });

			leds->on(RGB_LEDs::Left3, { levels.red, 0, 0 });
			leds->on(RGB_LEDs::Right3, { levels.red, 0, 0 });
			break;
		}

		case State::ShortBlink:
			leds->set(RGB_LEDs::Left3, newObject<LEDBlinkFunction<RGB_LEDs, glm::vec3>>(
					leds.get(), glm::vec3{ MaxBrightness, 0, 0 }, ShortBlinkInterval, BlinkOnDuration, 0));
			leds->set(RGB_LEDs::Right3, newObject<LEDBlinkFunction<RGB_LEDs, glm::vec3>>(
					leds.get(), glm::vec3{ MaxBrightness, 0, 0 }, ShortBlinkInterval, BlinkOnDuration, 0));
			break;

		case State::Longblink:
			leds->set(RGB_LEDs::Left3, newObject<LEDBlinkFunction<RGB_LEDs, glm::vec3>>(
					leds.get(), glm::vec3{ MaxBrightness, 0, 0 }, LongBlinkInterval, BlinkOnDuration, 0));
			leds->set(RGB_LEDs::Right3, newObject<LEDBlinkFunction<RGB_LEDs, glm::vec3>>(
					leds.get(), glm::vec3{ MaxBrightness, 0, 0 }, LongBlinkInterval, BlinkOnDuration, 0));
			break;
	}
}
