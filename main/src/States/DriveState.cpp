#include "DriveState.h"
#include "Enums.h"
#include "IntroState.h"
#include <Periphery/WiFi.h>
#include <Services/Battery.h>
#include <Services/ShutdownService.h>
#include <Services/TCPClient.h>
#include <Services/Audio/Audio.h>
#include <Services/Motors/Motors.h>
#include <Services/Motors/Servos.h>
#include <Services/LED/LED.h>

DriveState::DriveState() : lastRec(millis()){
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

	Feed* feed = app->getService<Feed>();
	if(feed == nullptr){
		return;
	}

	feed->setWorking(true);

	TCPClient* client = app->getService<TCPClient>();
	if(client == nullptr){
		return;
	}

	client->OnDisconnect.bind(this, &DriveState::onDisconnect);

	auto ledService = getApp()->getService<LED<LEDs, RGB_LEDs>>();
	ledService->on(LEDs::HeadlightsLeft, 1.0f);
	ledService->on(LEDs::HeadlightsRight, 1.0f);
	ledService->on(LEDs::TaillightsLeft, 1.0f);
	ledService->on(LEDs::TaillightsRight, 1.0f);
}

void DriveState::update() {
	if(millis() - lastRec >= ShutdownService::InactivityTimeout) {
		ShutdownService::shutdown(ShutdownReason::Inactivity);
		vTaskDelay(portMAX_DELAY);
		return;
	}

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

void DriveState::onTransitionTo(const Class* next) noexcept {
	Super::onTransitionTo(next);

	const Application* app = getApp();
	if(app == nullptr){
		return;
	}

	Feed* feed = app->getService<Feed>();
	if(feed == nullptr){
		return;
	}

	feed->setWorking(false);
}

void DriveState::onDisconnect() noexcept {
	if(transitionTo() != nullptr) {
		return;
	}

	if(auto audio = getApp()->getService<Audio>()){
		audio->play("/spiffs/Disconnect.aac");
	}

	auto ledService = getApp()->getService<LED<LEDs, RGB_LEDs>>();
	ledService->on(LEDs::HeadlightsLeft, 1.0f);
	ledService->on(LEDs::HeadlightsRight, 1.0f);
	ledService->on(LEDs::TaillightsLeft, 1.0f);
	ledService->on(LEDs::TaillightsRight, 1.0f);

	transition(IntroState::staticClass());
}

void DriveState::onSpeedReceived(float value) noexcept {
	TRACE_LOG("%llu", millis());
	lastRec = millis();

	const Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Motors<MotorsEnum>* motors = app->getService<Motors<MotorsEnum>>();
	if(motors == nullptr) {
		return;
	}

	motors->set(MotorsEnum::Motor, value * -1.0f);
}

void DriveState::onDirectionReceived(float value) noexcept {
	TRACE_LOG("%llu", millis());
	lastRec = millis();

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
