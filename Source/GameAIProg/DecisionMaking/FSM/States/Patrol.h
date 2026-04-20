#pragma once
#include "FSMState.h"

namespace GameAI::FSM
{
	class Patrol : public State
	{
	public:
		virtual void Tick(float deltaTime) override;
	};
}
