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
	Feed(StrongObjectPtr<Camera> camera = nullptr, StrongObjectPtr<Comm> comm = nullptr, StrongObjectPtr<UDPEmitter> udp = nullptr);


private:
	void tick(float deltaTime) noexcept override;

	void sendFrame(camera_fb_t* frameData);

	static constexpr size_t MaxJPEGBufSize = 9 * 1024;
	FeedFrame frame;


	StrongObjectPtr<Camera> camera;
	StrongObjectPtr<Comm> comm;
	StrongObjectPtr<UDPEmitter> udp;
};


#endif //NEVERA_FIRMWARE_FEED_H
