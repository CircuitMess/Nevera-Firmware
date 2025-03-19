#ifndef NEVERA_FIRMWARE_INTROSTATE_H
#define NEVERA_FIRMWARE_INTROSTATE_H

#include "Util/NevState.h"
#include <Services/ButtonInput.h>

class IntroState : public NevState {
	GENERATED_BODY(IntroState, NevState)

public:
	IntroState() noexcept;

private:
	void update() override;

	void onPress(Enum<int> button, ButtonInput::Action action) noexcept;

	uint32_t startMillis = 0;
};

#endif //NEVERA_FIRMWARE_INTROSTATE_H