#include "UDPEmitter.h"
#include "CommData.h"
#include <Log/Log.h>

DEFINE_LOG(UDPEmitter)

UDPEmitter::UDPEmitter() noexcept{
	socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(socket == -1){
		CMF_LOG(UDPEmitter, Error, "Can't create socket, errno=%d: %s", errno, strerror(errno));
		return;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(UDPPort);
	inet_pton(AF_INET, ControllerIP, &dest.sin_addr);
}

UDPEmitter::~UDPEmitter() noexcept{
	close(socket);
}

bool UDPEmitter::write(const std::vector<uint8_t>& buffer) noexcept{
	return write(buffer.data(), buffer.size());
}

bool UDPEmitter::write(const uint8_t* data, size_t count){
	if(socket == -1){
		CMF_LOG(UDPEmitter, Warning, "Write, but socket not set-up");
		return false;
	}

	if(!data || !count){
		CMF_LOG(UDPEmitter, Warning, "Write, but data or count zero");
		return true;
	}

	size_t total = 0;
	while(total < count){
		const int now = ::sendto(socket, data + total, count - total, 0, reinterpret_cast<sockaddr*>(&dest), sizeof(dest));

		if(now == 0){
			return false;
		}

		if(now < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				vTaskDelay(1);
				continue;
			}

			CMF_LOG(UDPEmitter, Warning, "Error %d", errno);

			return false;
		}

		total += now;
	}

	return true;
}
