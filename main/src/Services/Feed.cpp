#include "Feed.h"
#include "Util/stdafx.h"
#include "img_converters.h"

DEFINE_LOG(Feed)

Feed::Feed() : Super(45, 12 * 1024, 7, 1){
	Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Camera* camera = app->getDevice<Camera>();
	if(camera == nullptr) {
		return;
	}

	buffer = (uint8_t*)heap_caps_malloc(MaxJPEGBufSize, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
}

Feed::~Feed(){
	if(buffer){
		free(buffer);
		buffer = nullptr;
	}
}

void Feed::tick(float deltaTime) noexcept {
	Super::tick(deltaTime);

	if(!working){
		return;
	}

	Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Camera* camera = app->getDevice<Camera>();
	if(camera == nullptr) {
		return;
	}

	Comm* comm = app->getService<Comm>();
	if(comm == nullptr) {
		return;
	}

	const bool wasCamOff = !camera->isInited();

	const esp_err_t err = camera->init();
	if(err != ESP_OK){
		comm->sendNoFeed(true);

		vTaskDelay(1000); // No need to constantly tick if there is no feed.
		return;
	}else{
		//init finally ok!
		if(wasCamOff && camera->isInited()){
			comm->sendNoFeed(false);
		}
	}

	camera_fb_t* frameData = camera->getFrame();
	if(frameData == nullptr || frameData->buf == nullptr || frameData->len == 0){
		CMF_LOG(Feed, LogLevel::Error, "Couldnt get frame");
		camera->releaseFrame();
		return;
	}

	sendFrame(frameData);
}

void Feed::sendFrame(camera_fb_t* frameData){
	if(!working){
		return;
	}

	if(frameData == nullptr){
		return;
	}

	Application* app = getApp();
	if(app == nullptr) {
		return;
	}

	Camera* camera = app->getDevice<Camera>();
	if(camera == nullptr) {
		return;
	}

	UDPEmitter* udp = app->getService<UDPEmitter>();
	if(udp == nullptr) {
		CMF_LOG(Feed, LogLevel::Warning, "UDPEmitter is invalid.");
		camera->releaseFrame();
		return;
	}

	size_t size = 0;
	uint8_t* out;

	frame2jpg(frameData, 15, &out, &size);

	if(out == nullptr){
		CMF_LOG(Feed, LogLevel::Warning, "Out buffer from frame2jpg is nullptr.");
		camera->releaseFrame();
		return;
	}

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

	udp->write(buffer, sendSize);

	free(out);
	camera->releaseFrame();
}
