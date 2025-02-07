#include <Core/EntryPoint.h>
#include <Periphery/WiFi.h>
#include "src/Services/WiFiStation.h"
#include "src/Services/TCPClient.h"

class TestApp : public Application {
	GENERATED_BODY(TestApp, Application)

protected:
	virtual void begin() noexcept override {
		Super::begin();

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		registerService<TCPClient>();
	}

	virtual void tick(float deltaTime) noexcept override {
		Super::tick(deltaTime);
	}

	virtual void onDestroy() noexcept override {
		Super::onDestroy();
	}
};

CMF_MAIN(TestApp)