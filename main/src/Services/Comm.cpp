#include "Comm.h"
#include <CommData.h>
#include "TCPClient.h"

Comm::Comm() noexcept : Super(20, 4 * 1024, 6, -1) {
    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    TCPClient* tcp = app->getService<TCPClient>();
    if(tcp == nullptr) {
        return;
    }

    tcp->OnConnect.bind(this, &Comm::onTCPConnected);
}

void Comm::sendBattery(float percent) noexcept {
    StrongObjectPtr<CommData> driveData = newObject<CommData>(this);
    driveData->dataType = CommData::DataType::Battery;
    driveData->value = percent;

    sendPacket(driveData.get());
}

void Comm::sendNoFeed(bool noFeed){
	StrongObjectPtr<CommData> driveData = newObject<CommData>(this);
	driveData->dataType = CommData::DataType::NoFeed;
	driveData->value = noFeed;

	sendPacket(driveData.get());
}

TickType_t Comm::getEventScanningTime() const noexcept {
    const Application* app = getApp();
    if(app == nullptr) {
        return portMAX_DELAY;
    }

    const TCPClient* tcp = app->getService<TCPClient>();
    if(tcp == nullptr) {
        return portMAX_DELAY;
    }

    if(tcp->isConnected()) {
        return portMAX_DELAY;
    }

    return 0;
}

void Comm::tick(float deltaTime) noexcept {
    Super::tick(deltaTime);

    const Application* app = getApp();
    if(app == nullptr) {
        return;
    }

    TCPClient* tcp = app->getService<TCPClient>();
    if(tcp == nullptr) {
        return;
    }

    if(!tcp->isConnected()) {
        return;
    }

    std::vector<uint8_t> buffer(sizeof(size_t));
    if(!tcp->read(buffer)) {
        return;
    }

    size_t size = 0;
    memcpy(&size, buffer.data(), sizeof(size_t));

    buffer.resize(size);
    if(!tcp->read(buffer)) {
        return;
    }

    StrongObjectPtr<CommData> data = objectFromByteArray<CommData>(buffer, this);
    if(!data.isValid()) {
        return;
    }

    if(data->dataType == CommData::DataType::Direction) {
        OnDirectionReceived.broadcast(data->value);
    }else if(data->dataType == CommData::DataType::Speed) {
        OnSpeedReceived.broadcast(data->value);
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

void Comm::onTCPConnected() noexcept {

}
