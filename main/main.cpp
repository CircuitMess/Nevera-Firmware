#include <Core/EntryPoint.h>
#include <Drivers/Output/OutputMCPWM.h>
#include <Services/Motors/Servos.h>
#include <Util/stdafx.h>
#include "HardwareConfig.h"
#include "Enums.h"

class Nevera : public Application {
	GENERATED_BODY(Nevera, Application, void)

public:
	Nevera() noexcept: Super(1000, 4 * 1024, 8, 0){}

protected:
	virtual void begin() noexcept override {
		HardwareConfiguration* config = registerSingleton<HardwareConfiguration>();

		auto mcpwm = registerDriver<OutputMCPWM>(config->getMcpwmPinDefs());

		std::vector<ServoDef<ServoEnum>> servoDefs = {
				{ ServoEnum::Steer, { 0.f, 1.f }, { mcpwm, 0 }}
		};
		auto servos = registerService<Servos<ServoEnum>>(servoDefs);

		constexpr uint32_t SweepDuration = 2000; // ms
		constexpr uint32_t Steps = 100;
		constexpr uint32_t StepDelay = SweepDuration / Steps;

		while(true){
			// Sweep left to right slowly (2 seconds).
			for(uint32_t i = 0; i <= Steps; ++i){
				servos->set(ServoEnum::Steer, (float) i / (float) Steps);
				delayMillis(StepDelay);
			}
			delayMillis(1000);

			// Sweep right to left slowly (2 seconds).
			for(uint32_t i = 0; i <= Steps; ++i){
				servos->set(ServoEnum::Steer, 1.0f - (float) i / (float) Steps);
				delayMillis(StepDelay);
			}
			delayMillis(1000);

			// Center the servo and wait 2 seconds.
			servos->set(ServoEnum::Steer, 0.5f);
			delayMillis(2000);
		}
	}
};

CMF_MAIN(Nevera)
