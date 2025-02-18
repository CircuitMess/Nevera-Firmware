#include "IntroState.h"
#include "Enums.h"
#include <Core/Application.h>
#include "PairState.h"

IntroState::IntroState() noexcept {
    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    ButtonInput* input = app->getService<ButtonInput>();
    if(input == nullptr) {
        return;
    }

    input->OnButtonEvent.bind(this, &IntroState::onPress);
}

void IntroState::onPress(Enum<int> button, ButtonInput::Action action) noexcept {
    if(button != Button::Pair || action != ButtonInput::Action::Press) {
        return;
    }

    if(transitionTo() != nullptr) {
        return;
    }

    transition(PairState::staticClass());
}
