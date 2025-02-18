#ifndef NEVERA_COMM_H
#define NEVERA_COMM_H

#include <Entity/AsyncEntity.h>
#include <Event/EventBroadcaster.h>

class Comm : public AsyncEntity {
    GENERATED_BODY(Comm, AsyncEntity)

public:
    DECLARE_EVENT(SpeedReceivedEvent, Comm, float);
    SpeedReceivedEvent OnSpeedReceived{this};

    DECLARE_EVENT(DirectionReceivedEvent, Comm, float);
    DirectionReceivedEvent OnDirectionReceived{this};

public:
    Comm() noexcept;

    void sendBattery(float percent) noexcept;

	void sendNoFeed(bool noFeed);

protected:
    virtual TickType_t getEventScanningTime() const noexcept override;

    virtual void tick(float deltaTime) noexcept override;

private:
    void sendPacket(Object* object) noexcept;

    void onTCPConnected() noexcept;
};

#endif //NEVERA_COMM_H