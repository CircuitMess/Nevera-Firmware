#ifndef NEVERA_FIRMWARE_BATTERYINDICATOR_H
#define NEVERA_FIRMWARE_BATTERYINDICATOR_H

#include <Entity/AsyncEntity.h>
#include "Battery.h"
#include "Services/LED/LED.h"
#include "Enums.h"
#include <cmath>

class BatteryIndicator : public AsyncEntity {
	GENERATED_BODY(BatteryIndicator, AsyncEntity)
public:
	BatteryIndicator(StrongObjectPtr<Battery> battery = nullptr, StrongObjectPtr<LED<LEDs, RGB_LEDs>> leds = nullptr);

protected:
	void tick(float deltaTime) noexcept override;

private:
	StrongObjectPtr<Battery> battery;
	StrongObjectPtr<LED<LEDs, RGB_LEDs>> leds;

	enum class State : uint8_t {
		Continuous, ShortBlink, Longblink
	} state = State::Continuous;
	void gotoState(State newState, uint8_t perc);

	struct BrightnessLevels {
		float red;
		float orange;
		float green;
	};

	static constexpr BrightnessLevels getLevels(float percent){
		percent = std::clamp(percent, 0.f, 100.f);

		float green = std::clamp(percent - 66.6f, 0.f, 33.3f) * 0.03f;
		float orange = std::clamp(percent - 33.3f, 0.f, 33.3f) * 0.03f;
		float red = std::clamp(percent, 0.f, 33.3f) * 0.03f;

		return { red * MaxBrightness, orange * MaxBrightness, green * MaxBrightness };
	}

	static constexpr float LongBlinkInterval = 2.0f; //[s]
	static constexpr float ShortBlinkInterval = 0.5f; //[s]
	static constexpr float BlinkOnDuration = 0.1f; //[s]

	static constexpr uint8_t LongBlinkThreshold = 15; //red only will long-blink below 15% battery
	static constexpr uint8_t ShortBlinkThreshold = 5; //red only will short-blink below 5% battery

	static constexpr float MaxBrightness = 0.2f;
	static constexpr float GreenFactor = 0.25f; //Green LEDs are visually brighter than red at same value;

	static constexpr uint32_t UpdatePeriod = 5000; //[ms] - 5s
};


#endif //NEVERA_FIRMWARE_BATTERYINDICATOR_H
