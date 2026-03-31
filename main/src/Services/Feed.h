#ifndef NEVERA_FIRMWARE_FEED_H
#define NEVERA_FIRMWARE_FEED_H

#include <Entity/AsyncEntity.h>
#include <Devices/Camera.h>
#include "Comm.h"
#include "UDPEmitter.h"
#include <FeedFrame.h>

class Feed : public AsyncEntity{
	GENERATED_BODY(Feed, AsyncEntity, void)

public:
	Feed();
	~Feed() override;

	inline void setWorking(bool value) {
		TRACE_LOG("");
		working = value;
	}

private:
	void tick(float deltaTime) noexcept override;

	void sendFrame(camera_fb_t* frameData);

	static constexpr size_t MaxJPEGBufSize = 10 * 1024;

	uint8_t* buffer = nullptr;
	bool working = false;
};

#endif //NEVERA_FIRMWARE_FEED_H
