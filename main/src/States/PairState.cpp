#include "PairState.h"
#include <Enums.h>
#include "DriveState.h"
#include "IntroState.h"
#include <Services/TCPClient.h>
#include <Services/WiFiStation.h>

PairState::PairState() noexcept {
    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    ButtonInput* input = app->getService<ButtonInput>();
    if(input == nullptr) {
        return;
    }

    input->OnButtonEvent.bind(this, &PairState::onButton);

    WiFiStation* wifi = app->getService<WiFiStation>();
    if(wifi == nullptr) {
        return;
    }

    wifi->OnStationEvent.bind(this, &PairState::onStationEvent);
    wifi->connect();
}

void PairState::update() {
    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    TCPClient* client = app->getService<TCPClient>();
    if(client == nullptr) {
        return;
    }

    WiFiStation* wifi = app->getService<WiFiStation>();
    if(wifi == nullptr) {
        return;
    }

    if(wifi->getState() == WiFiStation::State::Connecting) {
        if(connectStart < 0) {
            connectStart = millis();
        }

        if(millis() - connectStart >= AbortTimeout) {
            wifi->disconnect();

            if(client->isConnected()) {
                client->disconnect();
            }

            wifi->connect();
        }
    }else {
        connectStart = -1;
    }
}

void PairState::onButton(Enum<int> button, ButtonInput::Action action) noexcept {
    if(button != Button::Pair || action != ButtonInput::Action::Release) {
        return;
    }

    if(transitionTo() != nullptr) {
        return;
    }

    transition(IntroState::staticClass());
}

void PairState::onStationEvent(WiFiStation::EventType type, bool success) noexcept {
    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    TCPClient* client = app->getService<TCPClient>();
    if(client == nullptr) {
        return;
    }

    WiFiStation* wifi = app->getService<WiFiStation>();
    if(wifi == nullptr) {
        return;
    }

    if(type == WiFiStation::EventType::Disconnect && client->isConnected()) {
        client->disconnect();
    }

    if(type != WiFiStation::EventType::Connect) {
        return;
    }

    if(success) {
        if(client->isConnected()) {
            client->disconnect();
        }

        const bool res = client->connect();
        if(!res) {
            wifi->disconnect();

            // TODO fail action, LED or whatever

            if(transitionTo() != nullptr) {
                return;
            }

            transition(IntroState::staticClass());
        }else {
            if(transitionTo() != nullptr) {
                return;
            }

            transition(DriveState::staticClass());
            return;
        }
    }else {
        if(attempted < ConnectionAttempts) {
            ++attempted;
            wifi->connect();
            return;
        }

        // TODO fail action, LED or whatever

        if(transitionTo() != nullptr) {
            return;
        }

        transition(DriveState::staticClass());
        return;
    }
}
