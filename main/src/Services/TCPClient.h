#ifndef NEVERA_TCPCLIENT_H
#define NEVERA_TCPCLIENT_H

#include "WiFiStation.h"
#include <Event/EventBroadcaster.h>
#include <Object/Object.h>

class TCPClient : public Object {
    GENERATED_BODY(TCPClient, Object, void)

public:
    DECLARE_EVENT(ConnectEvent, TCPClient);
    ConnectEvent OnConnect{this};

    DECLARE_EVENT(DisconnectEvent, TCPClient);
    DisconnectEvent OnDisconnect{this};

public:
    TCPClient() noexcept;

    bool isConnected() const noexcept;

    bool connect() noexcept;
    void disconnect() noexcept;

    bool read(std::vector<uint8_t>& buffer) noexcept;
    bool write(const std::vector<uint8_t>& buffer) noexcept;

    bool read(uint8_t* buffer, size_t count) noexcept;
    bool write(uint8_t* buffer, size_t count) noexcept;

private:
    int socket = -1;

private:
    void onStationEvent(WiFiStation::EventType eventType, bool success) noexcept;
};

#endif //NEVERA_TCPCLIENT_H