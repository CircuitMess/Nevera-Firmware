#include "Feed.h"
#include <utility>
#include "Util/stdafx.h"

DEFINE_LOG(Feed)

Feed::Feed(Camera* camera, Comm* comm, UDPEmitter* udp) :
		AsyncEntity(50, 4 * 1024), camera(camera), comm(comm), udp(udp){

	heapRep("feed constructor");
	buffer = (uint8_t*)heap_caps_malloc(MaxJPEGBufSize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	camera->setFormat(PIXFORMAT_JPEG);
}

Feed::~Feed(){
	heapRep("feed destructor");
	if(buffer){
		free(buffer);
		buffer = nullptr;
	}
}

void Feed::tick(float deltaTime) noexcept{
	AsyncEntity::tick(deltaTime);

	if(camera == nullptr){
		return;
	}

	const bool wasCamOff = !camera->isInited();

	const esp_err_t err = camera->init();
	if(err != ESP_OK){
		if(comm) comm->sendNoFeed(true);

		vTaskDelay(1000); // No need to constantly tick if there is no feed.
		return;
	}else{
		//init finally ok!
		if(wasCamOff && camera->isInited()){
			if(comm) comm->sendNoFeed(false);
		}
	}

	camera_fb_t* frameData = camera->getFrame();
	if(frameData == nullptr || frameData->buf == nullptr || frameData->len == 0){
		CMF_LOG(Feed, LogLevel::Warning, "Couldnt get frame");
		camera->releaseFrame();
		return;
	}

	sendFrame(frameData);
}

void Feed::sendFrame(camera_fb_t* frameData){
	CMF_LOG(Feed, LogLevel::Warning, "sendFrame");

	size_t size = frameData->len;
	uint8_t* out = frameData->buf;

	CMF_LOG(Feed, LogLevel::Info, "frame size (in JPEG): %d", size);

	const size_t frameSize = size;
	const size_t sendSize = frameSize + sizeof(FeedFrame::Header) + sizeof(FeedFrame::Trailer) + sizeof(size_t) * 2;

	if(sendSize > MaxJPEGBufSize){
		CMF_LOG(Feed, LogLevel::Warning, "Data frame buffer larger than send buffer. %zu > %zu\n", sendSize, MaxJPEGBufSize);
		camera->releaseFrame();
		return;
	}

	size_t cursor = 0;
	auto addData = [&cursor, this](const void* data, size_t size){
		memcpy(buffer + cursor, data, size);
		cursor += size;
	};

	uint8_t shiftedFrame[4];
	for(uint8_t i = 0; i < 4; i++){
		shiftedFrame[FeedFrame::SizeShift[i]] = ((uint8_t*) &frameSize)[i];
	}
	addData(FeedFrame::Header, sizeof(FeedFrame::Header));
	addData(&frameSize, sizeof(size_t));
	addData(shiftedFrame, sizeof(size_t));
	addData(out, size);
	addData(FeedFrame::Trailer, sizeof(FeedFrame::Trailer));

	size_t sent = 0;
	while(sent < sendSize){
		const size_t sending = std::min((size_t) CONFIG_TCP_MSS, sendSize - sent);
		bool ret = udp->write(buffer + sent, sending);
		printf("ret: %d\n", ret);
		sent += sending;
	}
	printf("sent\n");

	camera->releaseFrame();
}
