#ifndef DRIVESTATE_H
#define DRIVESTATE_H

#include "Util/NevState.h"
#include "Services/Feed.h"

class DriveState : public NevState{
    GENERATED_BODY(DriveState, NevState)

public:
	DriveState();
	// TODO listen to comm events and do appropriate stuff with them

private:
	StrongObjectPtr<Feed> feed;
};

#endif //DRIVESTATE_H
