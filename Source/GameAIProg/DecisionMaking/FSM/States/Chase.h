#pragma once
#include "FSMState.h"

namespace GameAI::FSM
{
	class Chase : public State
	{
	public:
		virtual void Tick(float deltaTime) override;
	};
}