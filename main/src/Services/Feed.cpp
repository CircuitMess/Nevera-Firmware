#include "Feed.h"
#include <utility>

DEFINE_LOG(Feed)

Feed::Feed(StrongObjectPtr<Camera> camera, StrongObjectPtr<Comm> comm, StrongObjectPtr<UDPEmitter> udp) :
		AsyncEntity(50, 12 * 1024), camera(std::move(camera)), comm(std::move(comm)), udp(std::move(udp)){

	frame.data.reserve(MaxJPEGBufSize);
	frame.header.reserve(FeedFrame::HeaderTrailerLength);
	frame.trailer.reserve(FeedFrame::HeaderTrailerLength);
	frame.shiftedSize.reserve(sizeof(size_t));

	camera->setFormat(PIXFORMAT_RGB565);
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
		camera->releaseFrame();
		return;
	}

	sendFrame(frameData);
}

void Feed::sendFrame(camera_fb_t* frameData){

	size_t size;
	uint8_t* out;

	if(!frame2jpg(frameData, 30, &out, &size)){
		CMF_LOG(Feed, LogLevel::Error, "frame2jpg conversion failed.");
		camera->releaseFrame();
		return;
	}

	frame.data = {out, out+size};
	free(out);

	const size_t sendSize = frame.data.size() + sizeof(FeedFrame::Header) + sizeof(FeedFrame::Trailer) + sizeof(size_t) * 2;

	if(size > MaxJPEGBufSize){
		CMF_LOG(Feed, LogLevel::Warning, "Data frame buffer larger than send buffer. %zu > %zu\n", sendSize, MaxJPEGBufSize);
		camera->releaseFrame();
		return;
	}

	frame.header = {FeedFrame::Header, FeedFrame::Header + FeedFrame::HeaderTrailerLength};
	frame.trailer = {FeedFrame::Trailer, FeedFrame::Trailer + FeedFrame::HeaderTrailerLength};

	const auto frameSize = frame.data.size();
	for(size_t i = 0; i < sizeof(size_t); i++){
		frame.shiftedSize[FeedFrame::SizeShift[i]] = ((uint8_t*) &frameSize)[i];
	}

	std::vector<uint8_t> udpData;
	byteArrayFromObject(&frame, udpData);

	std::vector<uint8_t> sizeData(sizeof(size_t));
	size = udpData.size();

	memcpy(sizeData.data(), &size, sizeof(size_t));


	if(udp){
		udp->write(sizeData);
		udp->write(udpData);
	}

	camera->releaseFrame();
}
