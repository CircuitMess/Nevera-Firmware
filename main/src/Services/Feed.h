#ifndef NEVERA_FIRMWARE_FEED_H
#define NEVERA_FIRMWARE_FEED_H

#include <Entity/AsyncEntity.h>
#include <Devices/Camera.h>
#include "Comm.h"
#include "UDPEmitter.h"
#include <FeedFrame.h>

class Feed : public AsyncEntity{
	GENERATED_BODY(Feed, AsyncEntity)

public:
	Feed();
	~Feed() override;

private:
	void tick(float deltaTime) noexcept override;

	void sendFrame(camera_fb_t* frameData);

	static constexpr size_t MaxJPEGBufSize = 10 * 1024;
	uint8_t* buffer = nullptr;
};


#endif //NEVERA_FIRMWARE_FEED_H
