#ifndef DRIVESTATE_H
#define DRIVESTATE_H

#include "Util/NevState.h"
#include "Services/Feed.h"

class DriveState : public NevState{
    GENERATED_BODY(DriveState, NevState)

public:
	DriveState();

	virtual void update();

private:
	StrongObjectPtr<Feed> feed;

private:
	void onDisconnect() noexcept;
	void onSpeedReceived(float value) noexcept;
	void onDirectionReceived(float value) noexcept;
};

#endif //DRIVESTATE_H
