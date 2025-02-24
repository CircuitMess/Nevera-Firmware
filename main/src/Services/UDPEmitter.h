#ifndef NEVERA_UDPEMITTER_H
#define NEVERA_UDPEMITTER_H

#include <lwip/sockets.h>
#include <Object/Object.h>

class UDPEmitter : public Object {
	GENERATED_BODY(UDPEmitter, Object)

public:
	UDPEmitter() noexcept;
	virtual ~UDPEmitter() noexcept override;

	bool write(const std::vector<uint8_t>& buffer) noexcept;

	bool write(const uint8_t* data, size_t count);

private:
	int socket = -1;
	sockaddr_in dest{};
};

#endif //NEVERA_UDPEMITTER_H