#ifndef NEVERA_COMM_H
#define NEVERA_COMM_H

#include <CommData.h>
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

    void sendConnection(float percent) noexcept;

protected:
    virtual void tick(float deltaTime) noexcept override;

private:
    StrongObjectPtr<CommData> data;
    StrongObjectPtr<CommData> sendData;

private:
    void sendPacket(Object* object) noexcept;
};

#endif //NEVERA_COMM_H