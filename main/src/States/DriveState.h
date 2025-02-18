#ifndef DRIVESTATE_H
#define DRIVESTATE_H

#include "Util/NevState.h"

class DriveState : public NevState{
    GENERATED_BODY(DriveState, NevState)

public:
    // TODO listen to comm events and do appropriate stuff with them
};

#endif //DRIVESTATE_H
