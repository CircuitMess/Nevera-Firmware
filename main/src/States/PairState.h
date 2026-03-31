#ifndef PAIRSTATE_H
#define PAIRSTATE_H

#include <Misc/Enum.h>
#include <Services/ButtonInput.h>
#include <Services/WiFiStation.h>
#include "Util/NevState.h"

class PairState : public NevState{
    GENERATED_BODY(PairState, NevState, void)

public:
    PairState() noexcept;

    virtual void update();

private:
    inline static constexpr const uint8_t ConnectionAttempts = 3;
    inline static constexpr const uint32_t AbortTimeout = 2000;

    uint8_t attempted = 1;
    int64_t connectStart = -1;

private:
    void onButton(Enum<int> button, ButtonInput::Action action) noexcept;
    void onStationEvent(WiFiStation::EventType type, bool success) noexcept;
};

#endif //PAIRSTATE_H
