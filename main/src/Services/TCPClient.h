#ifndef NEVERA_TCPCLIENT_H
#define NEVERA_TCPCLIENT_H

#include <Event/EventBroadcaster.h>
#include <Object/Object.h>

class TCPClient : public Object {
    GENERATED_BODY(TCPClient, Object)

public:
    DECLARE_EVENT(ConnectEvent, TCPClient);
    ConnectEvent OnConnect{this};

    DECLARE_EVENT(DisconnectEvent, TCPClient);
    DisconnectEvent OnDisconnect{this};

public:
    bool isConnected() const noexcept;

    bool connect() noexcept;
    void disconnect() noexcept;

    bool read(std::vector<uint8_t>& buffer) noexcept;
    bool write(std::vector<uint8_t>& buffer) noexcept;

private:
    int socket = -1;
};

#endif //NEVERA_TCPCLIENT_H