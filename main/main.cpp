#include <nvs_flash.h>
#include <Core/EntryPoint.h>
#include <Periphery/WiFi.h>
#include "src/Services/WiFiStation.h"
#include "src/Services/TCPClient.h"
#include "src/Services/UDPEmitter.h"

class TestApp : public Application {
	GENERATED_BODY(TestApp, Application)

protected:
	virtual void begin() noexcept override {
		Super::begin();

		auto ret = nvs_flash_init();
		if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);

		registerPeriphery<WiFi>();
		registerService<WiFiStation>();
		registerService<TCPClient>();
		registerService<UDPEmitter>();
	}

	virtual void tick(float deltaTime) noexcept override {
		Super::tick(deltaTime);
	}

	virtual void onDestroy() noexcept override {
		Super::onDestroy();
	}
};

CMF_MAIN(TestApp)