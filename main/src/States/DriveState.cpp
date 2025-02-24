#include "DriveState.h"
#include "Services/ButtonInput.h"

DriveState::DriveState(){

	printf("drivestate constr\n");

	const Application* app = getApp();
	assert(app != nullptr);

	auto camera = app->getDevice<Camera>();
	assert(camera != nullptr);
	auto comm = app->getService<Comm>();
	assert(comm != nullptr);
	auto udp = app->getService<UDPEmitter>();
	assert(udp != nullptr);

//	printf("cam: %p, comm: %p, udp: %p\n", camera, comm, udp);


	//DEBUG
	feed = newObject<Feed>(this, camera, comm, udp);

}
