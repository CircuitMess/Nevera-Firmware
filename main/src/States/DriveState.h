#ifndef DRIVESTATE_H
#define DRIVESTATE_H

#include "Util/NevState.h"
#include "Services/Feed.h"

class DriveState : public NevState{
    GENERATED_BODY(DriveState, NevState, void)

public:
	DriveState();

	virtual void update() override;

	virtual void onTransitionTo(const Class* next) noexcept override;

private:
	uint64_t lastRec;

private:
	void onDisconnect() noexcept;
	void onSpeedReceived(float value) noexcept;
	void onDirectionReceived(float value) noexcept;
};

#endif //DRIVESTATE_H
