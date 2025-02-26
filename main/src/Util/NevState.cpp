#include "NevState.h"

const Class* NevState::transitionTo() const noexcept{
	return nextState;
}

void NevState::transition(const Class* next) noexcept{
	CMF_LOG(CMF, LogLevel::Info, "state transition to %s", next->getName().c_str());
	nextState = next;
}

void NevState::tick(float deltaTime) noexcept{
	SyncEntity::tick(deltaTime);

	if(nextState) return;

	update();
}

void NevState::update(){

}
