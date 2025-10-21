#include "Comm.h"
#include "TCPClient.h"

Comm::Comm() noexcept : Super(12, 4 * 1024, 12, 0) {
    data = newObject<CommData>(this);
    sendData = newObject<CommData>(this);
}

void Comm::sendBattery(float percent) noexcept {
    sendData->dataType = CommData::DataType::Battery;
    sendData->value = percent;

    sendPacket(sendData.get());
}

void Comm::sendNoFeed(bool noFeed){
	sendData->dataType = CommData::DataType::NoFeed;
	sendData->value = noFeed;

	sendPacket(sendData.get());
}

void Comm::sendConnection(float percent) noexcept {
    sendData->dataType = CommData::DataType::Connection;
    sendData->value = percent;

    sendPacket(sendData.get());
}

void Comm::tick(float deltaTime) noexcept {
    Super::tick(deltaTime);

    const Application* app = getApp();
    if(app == nullptr) {
        vTaskDelay(100);
        return;
    }

    TCPClient* tcp = app->getService<TCPClient>();
    if(tcp == nullptr) {
        vTaskDelay(100);
        return;
    }

    if(!tcp->isConnected()) {
        TRACE_LOG("");
        vTaskDelay(100);
        return;
    }

    std::vector<uint8_t> buffer(sizeof(size_t));
    if(!tcp->read(buffer)) {
        TRACE_LOG("fail read");
        return;
    }

    size_t size = 0;
    memcpy(&size, buffer.data(), sizeof(size_t));

    buffer.resize(size);

    TRACE_LOG("%llu", millis());

    if(!tcp->read(buffer)) {
        return;
    }

    TRACE_LOG("%llu", millis());

    if(!objectFromByteArray(data.get(), buffer)) {
        return;
    }

    if(data->dataType == CommData::DataType::Direction) {
        OnDirectionReceived.broadcast(data->value);
        TRACE_LOG("%llu", millis());
    }else if(data->dataType == CommData::DataType::Speed) {
        OnSpeedReceived.broadcast(data->value);
        TRACE_LOG("%llu", millis());
    }
}

void Comm::sendPacket(Object *object) noexcept {
    if(object == nullptr){
        return;
    }

    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    TCPClient* tcp = app->getService<TCPClient>();
    if(tcp == nullptr) {
        return;
    }

    std::vector<uint8_t> data;
    byteArrayFromObject(object, data);

    std::vector<uint8_t> sizeData(sizeof(size_t));
    size_t size = data.size();

    memcpy(sizeData.data(), &size, sizeof(size_t));

    tcp->write(sizeData);
    tcp->write(data);
}
