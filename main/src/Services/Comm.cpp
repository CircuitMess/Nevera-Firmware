#include "Comm.h"
#include <CommData.h>
#include "TCPClient.h"

Comm::Comm() noexcept {
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

    StrongObjectPtr<DriveData> data = objectFromByteArray<DriveData>(buffer);
    if(!data.isValid()) {
        return;
    }

    if(data->dataType == DriveData::DataType::Direction) {
        OnDirectionReceived.broadcast(data->value);
    }else if(data->dataType == DriveData::DataType::Speed) {
        OnSpeedReceived.broadcast(data->value);
    }
}

void Comm::onTCPConnected() noexcept {

}
