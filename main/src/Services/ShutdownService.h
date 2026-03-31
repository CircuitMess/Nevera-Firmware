#ifndef NEVERA_FIRMWARE_SHUTDOWNSERVICE_H
#define NEVERA_FIRMWARE_SHUTDOWNSERVICE_H

#include <Object/Object.h>
#include <esp_sleep.h>
#include <Core/Application.h>


enum class ShutdownReason : uint8_t {
	Inactivity, Battery
};

class ShutdownService : public Object {
	GENERATED_BODY(ShutdownService, Object, void)
public:
	static constexpr uint32_t InactivityTimeout = 2 * 60000; //[ms] = 2 mins

	//Hardware shutdown, with notification audio beforehand
	static void shutdown(ShutdownReason reason);
};


#endif //NEVERA_FIRMWARE_SHUTDOWNSERVICE_H
