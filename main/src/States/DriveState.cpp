#include "DriveState.h"
#include "Enums.h"
#include "IntroState.h"
#include "Services/Audio/Audio.h"
#include <Services/TCPClient.h>
#include <Services/Motors/Motors.h>
#include <Services/Motors/Servos.h>

DriveState::DriveState(){
	const Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	auto comm = app->getService<Comm>();
	if(comm == nullptr) {
		return;
	}

	comm->OnSpeedReceived.bind(this, &DriveState::onSpeedReceived);
	comm->OnDirectionReceived.bind(this, &DriveState::onDirectionReceived);

	feed = newObject<Feed>(this);

	TCPClient* client = app->getService<TCPClient>();
	if(client == nullptr){
		return;
	}

	client->OnDisconnect.bind(this, &DriveState::onDisconnect);
}

void DriveState::update() {
	// TODO send battery state to the ctrl
}

void DriveState::onDisconnect() noexcept {
	if(transitionTo() != nullptr) {
		return;
	}

	if(auto audio = getApp()->getService<Audio>()){
		audio->play("/spiffs/Disconnect.aac");
	}

	transition(IntroState::staticClass());
}

void DriveState::onSpeedReceived(float value) noexcept {
	const Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Motors<MotorsEnum>* motors = app->getService<Motors<MotorsEnum>>();
	if(motors == nullptr) {
		return;
	}

	motors->set(MotorsEnum::Motor, value);
}

void DriveState::onDirectionReceived(float value) noexcept {
	const Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Servos<ServoEnum>* servos = app->getService<Servos<ServoEnum>>();
	if(servos == nullptr) {
		return;
	}

	servos->set(ServoEnum::Steer, value);
}
