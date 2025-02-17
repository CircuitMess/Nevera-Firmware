#ifndef NEVERA_CONTROLLER_SCREEN_H
#define NEVERA_CONTROLLER_SCREEN_H

#include <Util/StateMachine/State.h>

class NevState : public State {
	GENERATED_BODY(NevState, State);

public:
	virtual ~NevState() = default;

	const Class* transitionTo() const noexcept override final;
	void tick(float deltaTime) noexcept override final;

protected:
	void transition(const Class* next) noexcept;

private:
	const Class* nextState = nullptr;

	virtual void update();

};

#endif //NEVERA_CONTROLLER_SCREEN_H
