#ifndef NEVERA_FIRMWARE_INTROSTATE_H
#define NEVERA_FIRMWARE_INTROSTATE_H

#include "Util/NevState.h"
#include <Services/ButtonInput.h>

class IntroState : public NevState {
	GENERATED_BODY(IntroState, NevState)

public:
	IntroState() noexcept;

private:
	void onPress(Enum<int> button, ButtonInput::Action action) noexcept;
};

#endif //NEVERA_FIRMWARE_INTROSTATE_H