#include "UDPEmitter.h"
#include <Log/Log.h>

DEFINE_LOG(UDPEmitter)

UDPEmitter::UDPEmitter() noexcept {
    socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(socket == -1) {
        CMF_LOG(UDPEmitter, Error, "Can't create socket, errno=%d: %s", errno, strerror(errno));
        return;
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(6001);
    inet_pton(AF_INET, "11.0.0.2", &dest.sin_addr);
}

UDPEmitter::~UDPEmitter() noexcept {
    close(socket);
}

bool UDPEmitter::write(const std::vector<uint8_t>& buffer) noexcept {
    if(socket == -1){
        CMF_LOG(UDPEmitter, Warning, "Write, but socket not set-up");
        return false;
    }

    if(buffer.empty()) {
        return true;
    }

    size_t total = 0;
    while(total < buffer.size()){
        const int now = ::sendto(socket, buffer.data() + total, buffer.size() - total, 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));

        if(now == 0){
            return false;
        }

        if(now < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                vTaskDelay(1);
                continue;
            }

            return false;
        }

        total += now;
    }

    return true;
}
