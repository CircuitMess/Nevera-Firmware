#include "DriveState.h"
#include "Enums.h"
#include "IntroState.h"
#include <Periphery/WiFi.h>
#include <Services/Battery.h>
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
	const Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Comm* comm = app->getService<Comm>();
	if(comm == nullptr) {
		return;
	}

	Battery* battery = app->getService<Battery>();
	if(battery == nullptr) {
		return;
	}

	comm->sendBattery(battery->getPerc() / 100.0f);

	WiFi* wifi = app->getPeriphery<WiFi>();
	if(wifi == nullptr) {
		return;
	}

	comm->sendConnection((100.0f + wifi->getConnectionRSSI()) / 100.0f);
}

void DriveState::onDisconnect() noexcept {
	if(transitionTo() != nullptr) {
		return;
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

	servos->set(ServoEnum::Steer, (value + 1.0f) / 2.0f);
}
