#ifndef NEVERA_FIRMWARE_HARDWARECONFIG_H
#define NEVERA_FIRMWARE_HARDWARECONFIG_H

#include <Misc/Singleton.h>
#include "Drivers/Interface/OutputDriver.h"
#include "Pins.hpp"

class HardwareConfiguration : public Singleton {
		GENERATED_BODY(HardwareConfiguration, Singleton)

public:
	uint8_t getAW9523Address() const noexcept { return AW9523Address; }
	const std::vector<OutputPinDef>& getAW9523Outputs() const noexcept { return AW9523Outputs; }

private:
	const uint8_t AW9523Address = 0x5b;

	//ports are not inverted since AW9523 led driver is a current source
	const std::vector<OutputPinDef> AW9523Outputs = {
			{ EXP_HEAD_L, false },
			{ EXP_HEAD_R, false },
			{ EXP_TAIL_L, false },
			{ EXP_TAIL_R, false }
	};

};

#endif //NEVERA_FIRMWARE_HARDWARECONFIG_H
